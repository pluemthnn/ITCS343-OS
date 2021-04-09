#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "common.h"
#include "common_threads.h"

#ifdef linux
#include <semaphore.h>
#elif __APPLE__
#include "zemaphore.h"
#endif

#define BUFFER_SIZE 5
int item_buffer[BUFFER_SIZE]; //the buffer
int counter;                  //buffer counter
int item_inthread;
int cooking_count = 0;

int max;
int *buffer;

int use = 0;
int fill = 0;

sem_t empty;
sem_t full;
sem_t mutex;

struct MENU
{
    int menu_id;
    char ingredient1[20];
    char ingredient2[20];
    char ingredient3[20];
    int prepaire1;
    int prepaire2;
    int prepaire3;
    char medthod1[20];
    char medthod2[20];
    char medthod3[20];
    int cooking1;
    int cooking2;
    int cooking3;
} set_menu[BUFFER_SIZE];

#define CMAX (10)
int consumers = 1;

const char *filename = "inputs.txt"; // Readfile
int orderid_type[50][2];              // collect [orderID],[orderType]
int order_count = 0;                 //count number of order

double Time = 0;
double tidTurnAroundTime = 0;
struct timespec start, finish;

void setup_menu(int order_id, int order_type, int j)
{
    //printf(" %d ", order_type);
        //printf(" + %d==?%d + ", order_id, orderid_type[i][1]);
        if (order_type == 1)
        {   
            //printf(" + %d + %d", order_id,j);
            set_menu[j].menu_id = order_id;
            strcpy(set_menu[j].ingredient1, "Bread");
            strcpy(set_menu[j].ingredient2, "Vegetable");
            strcpy(set_menu[j].ingredient3, "Meat");
            set_menu[j].prepaire1 = 3;
            set_menu[j].prepaire2 = 5;
            set_menu[j].prepaire3 = 2;
            strcpy(set_menu[j].medthod1, "WAIT");
            strcpy(set_menu[j].medthod2, "WAIT");
            strcpy(set_menu[j].medthod3, "GRILL");
            set_menu[j].cooking1 = 1;
            set_menu[j].cooking2 = 1;
            set_menu[j].cooking3 = 6;
        }
        else if (order_type == 2)
        {
            set_menu[j].menu_id = order_id;
            strcpy(set_menu[j].ingredient1, "Noodle");
            strcpy(set_menu[j].ingredient2, "Broth");
            strcpy(set_menu[j].ingredient3, "Tempura");
            set_menu[j].prepaire1 = 5;
            set_menu[j].prepaire2 = 4;
            set_menu[j].prepaire3 = 5;
            strcpy(set_menu[j].medthod1, "WAIT");
            strcpy(set_menu[j].medthod2, "WAIT");
            strcpy(set_menu[j].medthod3, "GRILL");
            set_menu[j].cooking1 = 3;
            set_menu[j].cooking2 = 3;
            set_menu[j].cooking3 = 5;
        }
        else if (order_type == 3)
        {
            set_menu[j].menu_id = order_id;
            strcpy(set_menu[j].ingredient1, "Egg");
            strcpy(set_menu[j].ingredient2, "Amaranth");
            strcpy(set_menu[j].ingredient3, "Ham");
            set_menu[j].prepaire1 = 3;
            set_menu[j].prepaire2 = 4;
            set_menu[j].prepaire3 = 4;
            strcpy(set_menu[j].medthod1, "FRIED");
            strcpy(set_menu[j].medthod2, "BOIL");
            strcpy(set_menu[j].medthod3, "GRILL");
            set_menu[j].cooking1 = 3;
            set_menu[j].cooking2 = 3;
            set_menu[j].cooking3 = 5;
        }
        else if (order_type == 4)
        {
            set_menu[j].menu_id = order_id;
            strcpy(set_menu[j].ingredient1, "Rice");
            strcpy(set_menu[j].ingredient2, "Vegetable");
            strcpy(set_menu[j].ingredient3, "Meat");
            set_menu[j].prepaire1 = 4;
            set_menu[j].prepaire2 = 3;
            set_menu[j].prepaire3 = 3;
            strcpy(set_menu[j].medthod1, "COOK");
            strcpy(set_menu[j].medthod2, "CUT");
            strcpy(set_menu[j].medthod3, "SOUS VIDE");
            set_menu[j].cooking1 = 3;
            set_menu[j].cooking2 = 2;
            set_menu[j].cooking3 = 4;
        }
}

int random_between(int min, int max)
{
    int r = (rand() / (float)RAND_MAX) * (max + 1) + min;
    return r > max ? max : r;
}

void read_file()
{
    FILE *in_file = fopen(filename, "r");
    if (!in_file)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    if (stat(filename, &sb) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    char order[50][10];

    char *file_contents = malloc(sb.st_size);
    printf("----------------- receive order -----------------\n");
    while (fscanf(in_file, "%[^\n ] ", file_contents) != EOF)
    {
        //printf("%s\n", file_contents);
        strcpy(order[order_count], file_contents);
        order_count++;
    }
    printf("  # number of order: %d #\n", order_count);
    fclose(in_file);

    // Translate menu to number[orderID][orderType] in array
    for (int i = 0; i < order_count; i++)
    {
        // Generate random ID for an order
        int id = random_between(1000, 9999);
        while(id == 9999){
            id = random_between(1000, 8888);
        }
        //printf("%s id>%d\n", order[i], id);
        if (strcmp(order[i], "burger") == 0)
        {
            orderid_type[i][0] = id;
            orderid_type[i][1] = 1;
        }
        else if (strcmp(order[i], "ramen") == 0)
        {
            orderid_type[i][0] = id;
            orderid_type[i][1] = 2;
        }
        else if (strcmp(order[i], "omelette") == 0)
        {
            orderid_type[i][0] = id;
            orderid_type[i][1] = 3;
        }
        else if (strcmp(order[i], "bibimbap") == 0)
        {
            orderid_type[i][0] = id;
            orderid_type[i][1] = 4;
        }
    }

    for (int i = 0; i < order_count; i++)
    {
        int order_id = orderid_type[i][0];
        int order_type = orderid_type[i][1];
        printf("ORDER ID:%d MENU:%s  (type %d) \n", order_id, order[i], order_type);
        setup_menu(order_id, order_type, i);
    }
    //exit(EXIT_FAILURE); //run check input
}

int remove_item(int *item)
{
    /* When the buffer is not empty remove the item
    and decrement the counter */
    if (counter > 0)
    {
        *item = item_buffer[(counter - 1)];
        counter--;
        return *item;
    }
    else
    { // Error buffer empty
        return -1;
    }
}

int insert_item(int item)
{
    //item is orderID
    //When the buffer is not full add the item and increment the counter
    if (counter < BUFFER_SIZE)
    {
        item_buffer[counter] = item;
        //printf("Set order %d in BUFFER\n", item_buffer[counter]);
        counter++;
        return 0;
    }
    else
    { //Error if the buffer is full
        return -1;
    }
}

void *producer(void *arg)
{
    for (int j = 0; j < order_count; j++)
    {
        Sem_wait(&empty);
        Sem_wait(&mutex);
        insert_item(orderid_type[j][0]);
        printf("<PRODUCER>: PUT <ORDER %d> TO BUFFER\n",orderid_type[j][0]);
        Sem_post(&mutex);
        Sem_post(&full);
    }
    int i;
    // end case
    for (i = 0; i < consumers; i++)
    {
        Sem_wait(&empty);
        Sem_wait(&mutex);
        insert_item(-1);
        Sem_post(&mutex);
        Sem_post(&full);
    }
    return NULL;
}
//ThreadID, orderID
void cooking(void *arg, int order_item)
{
    long tid;
    tid = (long)arg;
    cooking_count++;

    for (int i = 0; i < order_count; i++)
    {
        //printf("<THREAD %ld>: <%d Cooking %d> \n", tid, order_item, set_menu[i].menu_id);
        if (order_item == set_menu[i].menu_id)
        {
            printf("<THREAD %ld>: <ORDER %d> %s %s\n", tid, set_menu[i].menu_id, "PREPARE", set_menu[i].ingredient1);
            sleep(set_menu[i].prepaire1);
            printf("<THREAD %ld>: <ORDER %d> %s %s\n", tid, set_menu[i].menu_id, "PREPARE", set_menu[i].ingredient2);
            sleep(set_menu[i].prepaire2);
            printf("<THREAD %ld>: <ORDER %d> %s %s\n", tid, set_menu[i].menu_id, "PREPARE", set_menu[i].ingredient3);
            sleep(set_menu[i].prepaire3);
            printf("<THREAD %ld>: <ORDER %d> %s %s\n", tid, set_menu[i].menu_id, set_menu[i].medthod1, set_menu[i].ingredient1);
            sleep(set_menu[i].cooking1);
            printf("<THREAD %ld>: <ORDER %d> %s %s\n", tid, set_menu[i].menu_id, set_menu[i].medthod2, set_menu[i].ingredient2);
            sleep(set_menu[i].cooking2);
            printf("<THREAD %ld>: <ORDER %d> %s %s\n", tid, set_menu[i].menu_id, set_menu[i].medthod3, set_menu[i].ingredient3);
            sleep(set_menu[i].cooking3);
        }
    }
    printf("<THREAD %ld>: <ORDER %d> %s \n", tid, order_item, "FINISH");
    //printf("<THREAD %ld>: <ORDER %d> %s \n", tid, order_item, "SERVE");
    clock_gettime(CLOCK_MONOTONIC, &finish);
    Time = (finish.tv_sec - start.tv_sec);
    Time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    tidTurnAroundTime += Time;
    printf("<THREAD %ld>: <ORDER %d> %s (timeuse:%f)\n", tid, order_item, "SERVE", Time);
    //printf("%f sec total -> %.2f\n", ppTime, tidTurnAroundTime); //time that thread use in each order
}
void *consumer(void *arg)
{
    int item;
    int order_item;
    long tid;
    tid = (long)arg;
    while (order_item != -1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        printf("<THREAD %ld>: %s\n", tid, "WAIT");
        Sem_wait(&full);
        Sem_wait(&mutex);
        order_item = remove_item(&item);
        Sem_post(&mutex);
        if(order_item != -1){
            printf("<THREAD %ld>: <ORDER %d> %s\n", tid, order_item, "START");
            cooking(arg, order_item);
        }
        Sem_post(&empty);
        //printf("%ld ---> get %d\n", tid, tmp);
        if (order_item == -1)
        {
            printf("<THREAD %ld>: %s\n", tid, "EXECUTE");
            break;
        }
    }
    
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <buffersize> <consumers>\n", argv[0]);
        // <buffersize> <consumers>
        // gcc -o npc povercooked.c -pthread && ./npc 3 5 

        exit(1);
    }
    max = atoi(argv[1]);
    consumers = atoi(argv[2]);
    assert(consumers <= CMAX);
    double average = 0;

    read_file(); //read input file
    counter = 0; //init buffer

    buffer = (int *)malloc(max * sizeof(int));
    assert(buffer != NULL);
    int i;
    for (i = 0; i < max; i++)
    {
        buffer[i] = 0;
    }

    Sem_init(&empty, max); // max are empty
    Sem_init(&full, 0);    // 0 are full
    Sem_init(&mutex, 1);   // mutex

    pthread_t pid, cid[CMAX];
    Pthread_create(&pid, NULL, producer, NULL);
    printf("----------------- Start -----------------\n");
    //clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < consumers; i++)
    {
        Pthread_create(&cid[i], NULL, consumer, (void *)(long long int)i);
    }
    Pthread_join(pid, NULL);
    for (i = 0; i < consumers; i++)
    {

        Pthread_join(cid[i], NULL);
    }

    //printf("%f %d\n",tidTurnAroundTime, order_count); check
    average = tidTurnAroundTime/order_count;
    printf("\nFINISH ALL ORDER\n");
    printf("AVERAGE TURNAROUND TIME: %f SEC\n", average);
    printf("\nExit the program\n");
    exit(0);
}
