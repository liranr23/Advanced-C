#include "Task.cpp"
#include <string>
#include <fstream>
#include "ResolverTask.cpp"
#include "ThreadPool.cpp"

using namespace std;

class RequestTask : public Task {
public:
    RequestTask(string input, ThreadPool* _tp) : Task(), filename(input) {
        tp = _tp;
    }
    ~RequestTask() {
        //you can add printing on exit(that how you can check these threads finishing AFTER the resolvers for the whole file) - add console_mutex lock in this case
    }
    //activating the task
    virtual void run() {
        FileToQueue(filename);     
        // this thread pool implementation doesn't delete
        // the tasks so we perform the cleanup here
        delete this;
    }
private:
    //opening the file - getting each hostname and setting it as a task for the resolver thread. printing output after it resolves each hostname
    void FileToQueue(string filename){
		ifstream input(filename.c_str());
		if (input.is_open()) {
			string line;
			while(getline(input,line))
			{  
                //getting the current array size
                int temp = arr_size;
                //locking the threads
                pthread_mutex_lock(&mutex);
                //checking for the condition(hostname resolved?)
                while(temp >= arr_size){
                    //adding the task to the resolvers. The safe queue of their thread pool is the shared resource here!!
                    tp->addTask(new ResolverTask(line));
                    //waiting to get resolved
                    pthread_cond_wait(&condition, &mutex);
                }
                //show current hostname result
                pthread_mutex_lock(&console_mutex);
                printf("%s", arr[arr_size - 1].c_str());
                pthread_mutex_unlock(&console_mutex);
                //unlock threads
                pthread_mutex_unlock(&mutex);
			}
			input.close();
		}
        //in case of bad filname
		else{
            pthread_mutex_lock(&console_mutex);
			fprintf(stderr, "bad input file path %s\n", filename.c_str());
            pthread_mutex_unlock(&console_mutex);
        }
	}
    //the input filename
    string filename;
    //the threadpool of the RESOLVERS
	ThreadPool *tp;
};
