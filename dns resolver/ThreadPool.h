#ifndef THREADPOOL_H
#define THREADPOOL_H
#include "PoolThread.cpp"
class SafeQueue;
class ThreadPool {
private:
	//this dynamic array will contains the threads
	PoolThread** threads;
	int th_size;
	//safequeue for the tasks
    SafeQueue queue;
public:
	//constructor with the number of threads
	ThreadPool(int n);
	//dtor
	~ThreadPool();
	//function to add a new task
	void addTask(Task *nt);
private:
	//cleanup function called from the dtor
	void finish();
};
#endif
