#include "utils.cpp"
#include <unistd.h>
#include <iostream>
#include <string>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <linux/limits.h>
#define KILL_CHILD 127
using namespace std;

//A function that prints the current path
void printCurPath(){
	char dir[PATH_MAX];
	string home, ret_val;
	ret_val = getcwd(dir, PATH_MAX);
	home = getenv("HOME");
	//manipulate the path with home to ~
	if(ret_val.compare(home) == 0){
        	ret_val = "~";
    	}
    	else if(ret_val.compare(0,home.length(),home) == 0){
        	ret_val.erase(0,home.length());
        	ret_val.insert(0,"~");
    	}
    	cout << "OS SHell:" << ret_val << '>';
}

//a function that execute command
void execute(cmdLine *pCmdLine){
	int id = 0;
	//forking the process
	int pid = fork();
	if(pid < 0){
		perror("fork");
		exit(1);
	}
	if(pid == 0){ /*child*/
		//executing, if there is an error exiting with kill child status(127)
		if((execvp(pCmdLine->arguments[0], pCmdLine->arguments)) < 0){
			perror("Error executing the command: ");
			_exit(KILL_CHILD);
		}
  	} /*end of child now the parent*/
	if((pCmdLine->blocking) != 1){
		//background process
		cout << "[" << pid << "]" << endl;
        	status = 0;
		//reaping zombies
		while(waitpid(-1, &status, WNOHANG) > 0);
	}
	if((pCmdLine->blocking) == 1){
		//reaping zombies, waiting for the child to end
		while(waitpid(-1, &status, WNOHANG) > 0);
		id = waitpid(-1, &status, 0);
		if (id < 0)
			perror("waitpid");
	}
	//changing the exit status
        if (WIFEXITED(status)) {
	    	status = WEXITSTATUS(status);
        }
	//termination signal exit, printing message(e.g killing proccess) 
        else if (WIFSIGNALED(status)) {
		status = WTERMSIG(status) + 128;
		cout << "[" << id << "]: exited, status=" << status << endl;
        }

}
//Function that gets the command, passing on which to execute
int choose_action(cmdLine *pCmdLine){
	//checking for exit option
	if(strcmp(pCmdLine->arguments[0], "exit") == 0){
		return 1;
	}
	//checking for cd option, executing it. And changing the exit status.
	else if(strcmp(pCmdLine->arguments[0], "cd") == 0){
		if((chdir(pCmdLine->arguments[1])) < 0){
			perror("cd: ");
			status = 1;
        	}else
        		status = 0;
		return 0;
  	}
	//other executions, calling execute function.
	else{
		execute(pCmdLine);
		return 0;
	}
}

int main (int argc , char* argv[]){
	cout << "Welcome to OS SHell!" << endl;
	cmdLine *command = NULL;
	const char * to_char = NULL;
	string input;
	while(1){
		//printing current path
        	printCurPath();
		//getting input(catching EOF)
		if(! getline(cin, input)){
			//clearing memory allocations
			freeCmdLines(command);
			cout << "C ya!" << endl;
			break;
		}
		to_char = input.c_str();
		//calling parser
		command = parseCmdLines(to_char);
		if(command != NULL){
			//calling to execute the command, if 1(exit) exiting.
			if((choose_action(command)) == 1){
				//clearing memory allocations
				freeCmdLines(command);
				cout << "C ya!" << endl;
				break;
			}
		}
		//reaping zombies - this one in case of not using execvp command
		while(waitpid(-1, &status, WNOHANG) > 0);
	}
	return 0;
}
