#ifndef TASK_H
#define TASK_H

//global mutex and condition for request-resolve task
static pthread_mutex_t mutex;
static pthread_cond_t condition;

//base task class
class Task {
public:
    Task() {}
    virtual ~Task() {}
    virtual void run()=0;
protected:
    
};
#endif