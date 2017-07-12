#include <pthread.h>
#include "RequestTask.cpp"
#include "ThreadPool.h"
#include "DumperThread.cpp"
#define MIN_ARGS 2
int main(int argc, char *argv[]) 
{
    //checking if there are enough arguments given(at least one for input and one for output)
    if(argc <= MIN_ARGS){
        cerr << "Not enough arguments! Exiting!" << endl;
        exit(1);
    }
    // Create a thread pools
    //NOTICE: you can change the number of thread in the pool as you wish - current is 10
    ThreadPool *tp = new ThreadPool(10);
	ThreadPool *tp2 = new ThreadPool(10);
    //initializing the mutex and condition(for the requester-resolver tasks)
    pthread_mutex_init(&mutex,0);
    pthread_cond_init(&condition, 0);

    //setting up the first thread pool with the filenames given
    for (int i = 1;i < argc - 1; ++i) {
        tp->addTask(new RequestTask(argv[i], tp2));
    }
    //clearing the threadpools after all the work is done
    delete tp;
	delete tp2;

    //checking if there is nothing to output to the file
    if(arr_size <= 0){
        cerr << "Nothing to output..Exiting" << endl;
        exit(1);
    }
    //creating the dumper thread with the given output file
    DumperThread dt(argv[argc - 1]);
    //activate the thread
    dt.run();
    //clearing the shared array of results
	delete[] arr;
    //cleanup global mutexes and condition
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&console_mutex); 
    printf("\nWork DONE!!!\n");
}
