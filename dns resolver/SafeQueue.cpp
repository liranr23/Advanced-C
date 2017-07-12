#include "SafeQueue.h"

SafeQueue::SafeQueue() {
    //initializing the mutex and condition
    pthread_mutex_init(&queue_def,0);
    pthread_cond_init(&wake_cond, 0);
}

SafeQueue::~SafeQueue() {
    //cleanup mutex and condition
    pthread_mutex_destroy(&queue_def);
    pthread_cond_destroy(&wake_cond);
}

Task* SafeQueue::nextTask() {
    Task *nt = 0;
    //lock the queue
    pthread_mutex_lock(&queue_def);
    //sleeping until a new task is added
    while (tasks.empty())
        pthread_cond_wait(&wake_cond, &queue_def);
    nt = tasks.front();
    tasks.pop();

    //unlock the queue
    pthread_mutex_unlock(&queue_def);
    return nt;
}
// Add a task
void SafeQueue::addTask(Task *nt) {
    //locking the queue
    pthread_mutex_lock(&queue_def);
    //adding a task
    tasks.push(nt);
    //wakeup-call for a new task to be done
    pthread_cond_signal(&wake_cond);
    //unlocking the queue
    pthread_mutex_unlock(&queue_def);
}
