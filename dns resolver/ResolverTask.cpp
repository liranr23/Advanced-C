#include "Task.cpp"
#include <string>
#include <iostream>
using namespace std;

// stdout mutex(also used for dumper write to file)
static pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;
//result array, shared dynamic array of results and its size
static string *arr = NULL;
static int arr_size = 0;
extern "C"
{
	#include "util.h"
}

class ResolverTask : public Task {
public:
    ResolverTask(string input) : Task(), hostname(input) {}
    ~ResolverTask() {
		//If you wish to add print when the thread ending the task(add console_mutex lock in this case).
	}
    virtual void run() {
        HostToIP();
        //cleanup the task
        delete this;
    }
private:
    void HostToIP(){
		//NOTICE: any number you wish can be here for max addresses possible to get
		int maxAddresses = 8;
		//creating the result array of dnslookup
		char** ipArray=new char*[maxAddresses];
		//the number of addresses given from the dnslookup
		int AddressesFound = 0;
		//allocating memory to the results(you can change the INET_ADDRSTRLEN)
		for(int i = 0;i < maxAddresses;++i)
        	ipArray[i]=new char[INET_ADDRSTRLEN];
		//locking the thread
		pthread_mutex_lock(&mutex);
		//reallocing the global result array
		string *temp = new string[arr_size + 1];
		//copying the data
		for(int i = 0; i < arr_size; ++i)
			temp[i] = arr[i];
		//releasing memory allocated by the old array
		delete[] arr;
		//setting the new one as the global one
		arr = temp;
		//increasing the global array size
		arr_size++;
		//string t is the result of the current resolve
		string result = hostname + ',';
		//calling to the dnslookupAll given within the utils(and in libmyutil.so)
		if(dnslookupAll(hostname.c_str(), ipArray, maxAddresses, &AddressesFound) == UTIL_SUCCESS)
		{
			//building up the result string
			for(int i = 0; i < AddressesFound - 1; ++i)
				result = result + ipArray[i] + ',';
			result = result + ipArray[AddressesFound - 1] + '\n';
			/*puting it in the global array of results
			NOTICE: you don't need to wait(this is a dynamic array so there is not size limit)!!!
			This is following Daniel Khankin answer on piazza: The shared memory segment should be dynamic.
			*/
			arr[arr_size - 1] = result;
		}
		//in case of error
		else
			//general result in case of error - puting it in the global array
			arr[arr_size - 1] = hostname + ',' + '\n';	
		//cleaning up memory allocated
		for(int i = 0;i < maxAddresses;i++)
			delete[] ipArray[i];
		delete[] ipArray;
		//unlock the threads
		pthread_mutex_unlock(&mutex);
		//waking up the request thread!!
		pthread_cond_signal(&condition);
	}
	//the resolver given hostname
    string hostname;
};
