#include "restaurant.h"

/*static counter for the readers-writers algo*/
static int reader_count = 0; 


int getShmid(const char* tok, char val, int size){
        key_t key;
        int shmid;
        /*creating key*/
        key = ftok(tok, val);
        if((shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|0666)) == -1)
        {
                /*THERE IS A BASH FILE INCLUDED (kill_ipcs.sh).
                you can run it to kill not needed ipcs!!!*/
                fprintf(stderr, "Shared memory segment exists - clean it before using(shmid: %d)\n",shmid);           
                if((shmid = shmget(key, size, 0)) == -1)
                {
                        perror("shmget");
                }
                exit(1);
        }
        return shmid;
}

char* getIPC(int shmid){
        char  *segptr;      
        /* Attach (map) the shared memory segment into the current process
        */
        if((segptr = (char*)shmat(shmid, 0, 0)) == NULL)
        {
                perror("shmat");
                exit(1);
        }
        return segptr;
}

void delShm(int shmid){
        //remove a shared memory section
        //IPC_RMID: Marks a segment for removal.
        shmctl(shmid, IPC_RMID, 0);
}

int initializeMenu(int menu_size){
        /*creating shmid for the menu*/
        int menu_shm = getShmid(".", 'A', menu_size * sizeof(Dish));
        /*arrays with data for the menu*/
        const char *names[MAX_MENU] = { "Pizza", "Salad", "Hamburger", "Spaghetti", "Pie", "Milkshake", "Lasagne", "French Fries", "Beer", "Cold Drink" };
        double prices[MAX_MENU] = { 10.00, 7.50, 12.00, 9.00, 9.50, 6.00, 11.50, 5.50, 5.00, 4.00 };
        Dish *menu = (Dish*)getIPC(menu_shm);
        /*inserting the menu*/
        for(int i = 0; i < menu_size; i++){
                menu[i].id = i;
                strcpy(menu[i].name, names[i]);
                menu[i].price = prices[i];
                menu[i].totalOrders = 0;
        }
        return menu_shm;
}

int initializeOrders(int numofCust){
        /*creating shmid for the orders board*/
        int order_shm = getShmid(".", 'B', numofCust*sizeof(Order));
        Order *orders = (Order*)getIPC(order_shm);
        /*initalizing the orders board*/
        for (int i = 0; i < numofCust; i++){
                orders[i].id = i;
                orders[i].itemId = 0;
                orders[i].amount = 0;
                orders[i].done = 1;   
        }
        return order_shm;
}

int initalizeCust(int numofCust){
        /*creating shmid for the customers*/
        int customers_shm = getShmid(".", 'C', numofCust*sizeof(bool));
        return customers_shm;
}

void printSimArgs(int sim_time, int menu_size, int numofCust, int numofWaiters){
        /*print function*/
        printf("=====Simulation arguments=====\n");
        printf("Simulation time: %d\n", sim_time); 
        printf("Menu items count: %d\n", menu_size);
        printf("Customers count: %d\n", numofCust);
        printf("Waiters count: %d\n", numofWaiters); 
        printf("==============================\n");
}

void printMenu(int menuShmid, int menu_size, bool res){
        /*print function*/
        Dish *dishIt = (Dish*)getIPC(menuShmid);
        int totalOrders = 0;
        double totalCash = 0;
        printf("==========Menu list===========\n");
        printf("%-3s%-13s%-7s%s\n", "Id","Name","Price","Orders");
        for(int i = 0; i < menu_size; i++){
                printf("%-3d%-13s%-7.2lf%d\n", dishIt[i].id + 1,dishIt[i].name,dishIt[i].price,dishIt[i].totalOrders);
                totalOrders += dishIt[i].totalOrders;
                totalCash += dishIt[i].totalOrders * dishIt[i].price;
        }
        printf("==============================\n");
        if(res)
                printf("Total orders %d, for an amount %.2lf NIL\n", totalOrders, totalCash);
}

void cleanUp(int menuShmid, int orderShmid, int customersShmid){
        /*calling delShm to remove the ipcs*/
        delShm(menuShmid);
        delShm(orderShmid);
        delShm(customersShmid);
}

int main(int argc, char*argv[]){
        /*checking the arguments amount*/
	if(argc != 5){
		fprintf(stderr,"Input arguments are not valid!\n");
		exit(1);
	}
        /*converting the arguments into int*/
        int sim_time = atoi(argv[1]);
        int menu_size = atoi(argv[2]);
        int numofCust = atoi(argv[3]);
        int numofWaiters = atoi(argv[4]);
        /*checking the arguments values*/
        if(sim_time <= MIN_ARG || sim_time > MAX_SIM || menu_size < MIN_MENU || menu_size > MAX_MENU || numofWaiters <= MIN_ARG || numofWaiters > MAX_WAITERS || numofCust <= MIN_ARG || numofCust > MAX_CUST){
                fprintf(stderr,"Input arguments are not valid!\n");
		exit(1);
        }
        /*creating the ipcs*/
        int menuShmid, orderShmid, customersShmid;
        menuShmid = initializeMenu(menu_size);
        orderShmid = initializeOrders(numofCust);
        customersShmid = initalizeCust(numofCust);

        /*creating semaphores
        mutex - general mutex for the reader count,
        mutex2 - anti collision waiters mutex(they can't read the same order),
        db - mutex for the menu lock,
        db2 - mutex for the orders board lock,
        outmutex - mutex for output prints
        */
        int mutex, mutex2, db, db2, outmutex;
        key_t semkeyDB = ftok(".", 1);
        key_t semkeyMutex = ftok(".", 2);
        key_t semkeyMutex2 = ftok(".", 3);
        key_t semkeyDB2 = ftok(".", 4);
        key_t semkeyoutMutex = ftok(".", 5);
        /*in case of error, cleaning up the ipcs made*/
        if( ( db = initsem(semkeyDB, 1) ) < 0 )
	{
                cleanUp(menuShmid, orderShmid, customersShmid);
		exit(1);
	}
        if( ( mutex = initsem(semkeyMutex, 1) ) < 0 )
	{
                semctl(db, 0, IPC_RMID, NULL); 
                cleanUp(menuShmid, orderShmid, customersShmid);
		exit(1);
	}
        if( ( db2 = initsem(semkeyDB2, 1) ) < 0 )
	{
                semctl(db, 0, IPC_RMID, NULL); 
                semctl(mutex, 0, IPC_RMID, NULL); 
                cleanUp(menuShmid, orderShmid, customersShmid);
		exit(1);
	}
        if( ( outmutex = initsem(semkeyoutMutex, 1) ) < 0 )
	{
                semctl(db, 0, IPC_RMID, NULL); 
                semctl(mutex, 0, IPC_RMID, NULL); 
                semctl(db2, 0, IPC_RMID, NULL); 
                cleanUp(menuShmid, orderShmid, customersShmid);
		exit(1);
	}
        if( ( mutex2 = initsem(semkeyMutex2, 1) ) < 0 )
	{
                semctl(db, 0, IPC_RMID, NULL); 
                semctl(mutex, 0, IPC_RMID, NULL); 
                semctl(db2, 0, IPC_RMID, NULL); 
                semctl(outmutex, 0, IPC_RMID, NULL); 
                cleanUp(menuShmid, orderShmid, customersShmid);
		exit(1);
	}

        printSimArgs(sim_time, menu_size, numofCust, numofWaiters);
        /*creating start time clock*/
        chrono::time_point<std::chrono::system_clock> start = chrono::high_resolution_clock::now();

        printf("%6.3lf Main process ID %d start\n", (double)0, getpid());
        printf("==============================\n");
        printMenu(menuShmid, menu_size, false);
        printf("%6.3lf Main process start creating sub-process\n", (double)0);
        int childPid;
        /*creating waiters*/
        for(int i = 0; i < numofWaiters ; i++ )
	{
		childPid = fork();
                if(childPid == -1){
                        perror("fork");
                        cleanUp(menuShmid, orderShmid, customersShmid);
                        semctl(mutex, 0, IPC_RMID, NULL); 
                        semctl(mutex2, 0, IPC_RMID, NULL); 
                        semctl(db, 0, IPC_RMID, NULL); 
                        semctl(db2, 0, IPC_RMID, NULL); 
                        semctl(outmutex, 0, IPC_RMID, NULL);
                        exit(1);
                }

		if( childPid == 0 )/*child*/
		{
                        /*calling to the waiter function*/
                        waiter(start, sim_time, i,menuShmid, orderShmid, numofCust, mutex, mutex2, db, db2, outmutex, customersShmid);
		}
                p(outmutex);
                printf("%6.3lf Waiter %d: created PID %d PPID %d\n", getTimeChrono(start), i, childPid, getpid());
                v(outmutex);
	}
        /*end creating waiters*/
        /*creating customers*/
        for(int i = 0; i < numofCust ; i++ )
	{
		childPid = fork();
                if(childPid == -1){
                        perror("fork");
                        cleanUp(menuShmid, orderShmid, customersShmid);
                        semctl(mutex, 0, IPC_RMID, NULL); 
                        semctl(mutex2, 0, IPC_RMID, NULL); 
                        semctl(db, 0, IPC_RMID, NULL); 
                        semctl(db2, 0, IPC_RMID, NULL); 
                        semctl(outmutex, 0, IPC_RMID, NULL);
                        exit(1);
                }

		if( childPid == 0 )/*child*/
		{
                        /*calling to the customer function*/
                        customer(start, sim_time, menuShmid, orderShmid, i, menu_size, mutex, db2, outmutex, customersShmid);
		}
                p(outmutex);
                printf("%6.3lf Customer %d: created PID %d PPID %d\n", getTimeChrono(start), i, childPid, getpid());
                v(outmutex);
	}

        /*waiting for every child to finish his work*/
        while(waitpid(-1, NULL, 0) >= 0);
        /*printing the menu and the simulation results*/
        printMenu(menuShmid, menu_size, true);

        /*ipcs cleanup*/
        cleanUp(menuShmid, orderShmid, customersShmid);
        semctl(mutex, 0, IPC_RMID, NULL); 
        semctl(mutex2, 0, IPC_RMID, NULL); 
        semctl(db, 0, IPC_RMID, NULL); 
        semctl(db2, 0, IPC_RMID, NULL); 
        semctl(outmutex, 0, IPC_RMID, NULL);

        printf("%6.3lf Main ID %d end work\n", getTimeChrono(start), getpid());
        printf("%6.3lf End of simulation\n", getTimeChrono(start));

        return 0;
}

double getTimeChrono(chrono::time_point<std::chrono::system_clock> start){
        chrono::time_point<std::chrono::system_clock> t2 = chrono::high_resolution_clock::now();
        chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - start);
        return time_span.count();
}

/*THIS FUNCTION IMPLEMENTS THE READERS-WRITERS ALGORITHM*/
Order checkOrder(int custId, int mutex, int db2, int orderShmid){
        Order *orderIt = (Order*)getIPC(orderShmid);
        Order myOrder;
        /*READING ORDER*/
        p(mutex);
        reader_count = reader_count + 1;
        if (reader_count == 1)
                p(db2);
        v(mutex);
        myOrder.id = orderIt[custId].id;
        myOrder.itemId = orderIt[custId].itemId;
        myOrder.amount = orderIt[custId].amount;
        myOrder.done = orderIt[custId].done;
        p(mutex);
        reader_count = reader_count - 1;
        if (reader_count == 0)
                v(db2);
        v(mutex);
        /*DONE READING THE ORDER*/
        return myOrder;
}

void customer(chrono::time_point<std::chrono::system_clock> start,int end_time, int menuShmid,int orderShmid, int custId, int menu_size, int mutex, int db2, int outmutex, int customersShmid){
        /*setup*/
        Dish *dishIt = (Dish*)getIPC(menuShmid);
        Order *orderIt = (Order*)getIPC(orderShmid);
        bool *custIt = (bool*)getIPC(customersShmid);
        /*setting the boolean to alive customer*/
        p(mutex);
        custIt[custId] = true;
        v(mutex);
        Dish *menu = new Dish[menu_size];
        int chosen_item, amount;
        /*randomize cpu clock*/
        srand(time(NULL) ^ (getpid()<<16));
        /*when the sim time didn't over*/
        while (getTimeChrono(start) < end_time) {
                /*READING THE MENU*/
                sleep(rand() % 3 + 3); 
                /*no need to lock because no writing is done and no critial info for the customer(doesn't care about the total orders)*/ 
                for(int i = 0; i < menu_size; i++){
                        menu[i].id = dishIt[i].id;
                        strcpy(menu[i].name, dishIt[i].name);
                        menu[i].price = dishIt[i].price;
                        menu[i].totalOrders = dishIt[i].totalOrders;
                }
                sleep(1);
                /*DONE READING THE MENU*/
                /*checking if the current order is done*/
                if(checkOrder(custId, mutex, db2, orderShmid).done){   
                        /*it's done - making a decision about the next order*/                    
                        chosen_item = rand() % menu_size;
                        if(rand() % 2){
                                /*YAY the customer want to order*/
                                /*WRITING TO ORDERS*/
                                amount = rand() % 4 + 1;
                                p(outmutex);
                                printf("%6.3lf Customer ID %d: read a menu about %s (ordered, amount %d)\n", getTimeChrono(start), custId, menu[chosen_item].name, amount);
                                v(outmutex);
                                /*locking the orders board for writing*/
                                p(db2);//orders
                                orderIt[custId].id = custId;
                                orderIt[custId].itemId = chosen_item;
                                orderIt[custId].amount = amount;
                                orderIt[custId].done = 0;
                                v(db2);/*FINISHED WRITING ORDER*/
                        }
                        else{
                                /*the customer decide not to make an order :( */
                                p(outmutex);
                                printf("%6.3lf Customer ID %d: read a menu about %s (doesn't want to order)\n", getTimeChrono(start), custId, menu[chosen_item].name);
                                v(outmutex);
                        }
                }
        }
        /*clearing the menu allocation*/
        delete[]menu;
        p(outmutex);
        printf("%6.3lf Customer ID %d: PID %d end work PPID %d\n", getTimeChrono(start), custId, getpid(), getppid());
        v(outmutex);
        /*setting the status of the customer to dead(ended work)*/
        p(mutex);
        custIt[custId] = false;
        v(mutex);
        exit(0);
}

bool endWaiter(int customersShmid, int numofCust, int mutex, int db2, int orderShmid){
        /*checking if the customers are done and every order is done*/
        bool *custIt = (bool*)getIPC(customersShmid);
        for(int i=0; i < numofCust; i++){
                if(!(checkOrder(i, mutex, db2, orderShmid).done) || custIt[i])
                        return true;
        }
        return false;
}

void waiter(chrono::time_point<std::chrono::system_clock> start,int end_time, int id, int menuShmid, int orderShmid, int numofCust, int mutex, int mutex2, int db, int db2, int outmutex, int customersShmid){
        /*setup*/
        Order *orderIt = (Order*)getIPC(orderShmid);
        Dish *dishIt = (Dish*)getIPC(menuShmid);
        Order taken;
        /*randomize cpu clock*/
        srand(time(NULL) ^ (getpid()<<16));
        taken.itemId = 0;
        taken.amount = 0;
        /*boolean to know if an order is taken*/
        bool take = false; 
        /*checking if the current order is done and the endWaiter function returns false(everything is done only waiters left alive)*/
        while (getTimeChrono(start) < end_time || endWaiter(customersShmid, numofCust,mutex,db2,orderShmid)) {
                sleep(rand()%2 + 1);                           
                /*READING ORDER*/
                /*mutex 2 let only one waiter to search and take an order.
                in case of not using it, it can cause 2(or more) waiters to take the same order*/
                p(mutex2);
                for(int i = 0; i < numofCust; i++){
                        /*checking if there is an untaken order - if there is, taking it*/
                        taken = checkOrder(i, mutex, db2, orderShmid);
                        if(!taken.done){
                                take = true;
                                break;
                        }
                }
                if(take){
                        /*there is an order to take care of*/
                        /*WRITING TO THE MENU-locking*/
                        p(db);
                        dishIt[taken.itemId].totalOrders = dishIt[taken.itemId].totalOrders + taken.amount;
                        v(db);
                        /*DONE WRITING TO THE MENU*/
                        /*WRITING TO ORDER-locking*/
                        p(db2);
                        orderIt[taken.id].done = 1;
                        v(db2);
                        /*DONE WRITING TO ORDERS*/
                        p(outmutex);
                        printf("%6.3lf Waiter ID %d: performs the order of customer ID %d (%d %s)\n", getTimeChrono(start), id, taken.id, taken.amount,dishIt[taken.itemId].name);
                        v(outmutex);
                }
                take = false;
                /*releasing the waiters*/
                v(mutex2);
        }
        p(outmutex);
        printf("%6.3lf Waiter ID %d: PID %d end work PPID %d\n", getTimeChrono(start), id, getpid(), getppid());
        v(outmutex);
        exit(0);
}
