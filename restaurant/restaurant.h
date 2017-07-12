#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip>
#include <sys/sem.h>
#include <sys/types.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <chrono>
/*Defines*/
#define SEMPERM 0600
#define MIN_MENU 5
#define MAX_MENU 10
#define MAX_WAITERS 3
#define MAX_CUST 10
#define MIN_ARG 0
#define MAX_SIM 30
using namespace std;

/*STRUCTS*/
/*Dish struct - represents a dish from the menu*/
typedef struct {
	int id, totalOrders;
    double price;
    char name[16];
} Dish; 

/*Order struct - represents an order of a customer*/
typedef struct {
	int id, itemId, amount, done;
} Order; 


/*a function that returns the shmid by the given token, val(for ftok) and size*/
int getShmid(const char* tok, char val, int size);

/*a function that returns the pointer to the ipc by the shmid*/
char* getIPC(int shmid);

/*a function that removes ipc by shmid*/
void delShm(int shmid);

/*a function that removes the 3ipcs made(by calling to delShm for each)*/
void cleanUp(int menuShmid, int orderShmid, int customersShmid);

/*a function that initalize the menu to the ipc*/
int initializeMenu(int menu_size);

/*a function that initalize the orders board to the ipc*/
int initializeOrders(int numofCust);

/*a function that prints the starting args given*/
void printSimArgs(int sim_time, int menu_size, int numofCust, int numofWaiters);

/*a function that prints the menu(res tell if to print the simulation results(how many orders and money made)*/
void printMenu(int menuShmid, int menu_size, bool res);

/*a function that returns the current time*/
double getTimeChrono(chrono::time_point<std::chrono::system_clock> start);

/*a function that reads and returns a requested order*/
Order checkOrder(int custId, int mutex, int db2, int orderShmid);

/*a function that checks if the customers finished thier work and no order left to be done*/
bool endWaiter(int customersShmid, int numofCust, int mutex, int db2, int orderShmid);

/*a function represents the customer algorithm*/
void customer(chrono::time_point<std::chrono::system_clock> start, int , int menuShmid, int orderShmid,int custId, int menu_size, int mutex, int db2, int outmutex, int customersShmid);

/*a function represents the waiter algorithm*/
void waiter(chrono::time_point<std::chrono::system_clock> start, int, int id, int menuShmid, int orderShmid, int numofCust, int mutex,int mutex2, int db, int db2, int outmutex, int customersShmid);

/*the init semaphore function*/
int initsem(key_t semkey, int initval)
{
	int status=0,semid;

	union semun 
	{
		int val;
		struct semid_ds *stat;
		ushort *array;
	} ctl_arg;
	if ( ( semid = semget(semkey, 1, SEMPERM | IPC_CREAT | IPC_EXCL) ) == -1 )
	{
		semid = semget(semkey, 1, 0);
	}
	else
	{
		ctl_arg.val = initval;
		status = semctl(semid, 0, SETVAL, ctl_arg);
	}

	if(semid == -1 || status == -1)
	{
		perror("initsem failed");
		return(-1);
	}
	else
	{
		return semid;
	}
}

/*the push semaphore function*/
int p(int semid)
{
	struct sembuf p_buf;
	p_buf.sem_num = 0;
	p_buf.sem_op = -1;
	p_buf.sem_flg = SEM_UNDO;

	if( semop( semid, &p_buf, 1) == -1 )
	{
		perror("p(semid) failed");
		exit(1);
	}
	else
	{
		return 0;
	}
}

/*the up/post semaphore function*/
int v(int semid)
{
	struct sembuf v_buf;
	v_buf.sem_num = 0;
	v_buf.sem_op = 1;
	v_buf.sem_flg = SEM_UNDO;

	if( semop( semid, &v_buf, 1 ) == -1 )
	{
		perror("v(semid) failed");
		exit(1);
	}
	else
	{
		return 0;
	}
} 