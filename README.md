# ITCS343-OS
## multi-threads project

#### Problem
We are going to let each thread be a player. All threads will take an order from a buffer, prepare and serve it. The program ends when all orders are served. We can see that this multithreaded program will need a synchronization where many players can rush in to take the same order. Thus we need your help to write this program.
#### Here is a list of requirements that will make this povercooked-normal easier to manage:
1. All threads can take only 1 order at a time.
2. One order can only be prepared by 1 thread.
3. Only 5 orders can be seen at a time. Serving an order will reveal the next one.
4. Maximum ingredients for all recipes are 3. So a maximum of 6 + 1 actions is required for each recipe (3 preparings, 3 cooking, 1 serving).
5. The orders will not expire. But the turnaround time should be minimized.
6. The sleep function is used in place of the time to prepare and cook an order (total time of
all steps).
