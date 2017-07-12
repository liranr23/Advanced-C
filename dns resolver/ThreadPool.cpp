#include "ThreadPool.h"
#include "stdio.h"
// Allocate a thread pool and set them to work trying to get tasks
ThreadPool::ThreadPool(int n):th_size(n) {
    //if you wish to see the ctor works uncomment the following line:
    //printf("Creating a thread pool with %d threads\n", n);
    threads = new PoolThread*[n];
    for (int i=0; i<n; ++i)
    {
        //creating the threads and starting them
        threads[i] = new PoolThread(queue);
        threads[i]->start();
    }
}

// Wait for the threads to finish, then delete them
ThreadPool::~ThreadPool() {
    finish();
}

// adding a task
void ThreadPool::addTask(Task *nt) {
    queue.addTask(nt);
} 
// waiting for the task queue to end and then cleaning up
void ThreadPool::finish() {
    for(int i=0; i < th_size;++i)
        queue.addTask(NULL);
    for(int i=0; i < th_size;++i){
        threads[i]->join();
        delete threads[i];
    } 
    delete[] threads;
}
