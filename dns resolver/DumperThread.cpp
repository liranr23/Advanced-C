#include "Task.cpp"
#include <iostream>
#include <fstream>
//this class is the dumper thread
class DumperThread : public Thread{
private:
    //output file given
    string output;
public:
    //running dumperthread task
    virtual void run(){
        //in case of error opening the output file you can drop the check and the programm will make the result file
        ifstream file(output.c_str());
        if(file.bad() || file.fail()){
            pthread_mutex_lock(&console_mutex);
            cerr << "Bad output file(" << output << "). You need to create it!" << endl;
            pthread_mutex_unlock(&console_mutex);
            return;
        }
        else
            file.close();

        //opening the file, printing the result from the shared array to the file
        ofstream myfile (output.c_str());
        if (myfile.is_open()){
            for(int i = 0; i < arr_size; i++){
                //defending the writing(using console mutex because no output to the console is used)
                pthread_mutex_lock(&console_mutex);
                myfile << arr[i];
                pthread_mutex_unlock(&console_mutex);
            }
            myfile.close();
        }
        //in case of error opening the output file
        else{
            pthread_mutex_lock(&console_mutex);
            cerr << "Bad output file(" << output << ")!" << endl;
            pthread_mutex_unlock(&console_mutex);
        }
    }
    DumperThread(string outfile) : output(outfile) {};
};