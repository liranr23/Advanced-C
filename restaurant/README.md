# Resaurant simulation

You can run the programm by the make command. If your shared memory(ipc) isn't clean, you might use kill_ipcs.sh script to clean it up.
The programm has a menu, waiters and customers.
you run the programm with the args:

arg1: Simulation time.

arg2: Menu items.

arg3: Customers count.

arg4: Waiters count.

The limits are up to 10 dishes(menu items), up to 3 waiters and up to 10 customers.

The restaurant implements the Reader-Writer problem, and Shared memory(ipc).
