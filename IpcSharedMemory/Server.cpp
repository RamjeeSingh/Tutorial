#include <iostream>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


using namespace std;


void client();
void server();

int main(){

	pid_t pid = -1;
	cout << "Server Main" << endl;

	if ( (pid = fork()) < 0) {
		cout << "Fork failed";
	} else if (pid > 0 ) {
		int chldStatus;
		cout << "Fork in parent process pid = " << pid << endl;
		server();
		cout << "Fork parent wait pid "<< endl;
		waitpid(pid, &chldStatus, 0);
		cout << "Fork parent wait done "<< endl;
	} else {
		cout << "Fork in child process pid = " << pid << endl;
		client();
	}
	return 0;
}


void server (){
	char * buff ;
	int handle;
	int i = 0;
	key_t key = ftok("ipcfile.txt", 99999);
	handle = shmget(key, 1024, 0666 | IPC_CREAT);
	while(i++ < 20) {
		buff = (char*) shmat(handle,(void*)0,0);
		memcpy(buff, "hello from server1" , strlen("hello from server 1"));

	}
//	shmdt(buff);

}
void client (){
	char * buff;
	int handle;
	int i = 0;
	key_t key = ftok("ipcfile.txt", 99999);
	handle = shmget(key, 1024, 0666 | IPC_CREAT);
	while(i++ < 20) {
		buff = (char*) shmat(handle,(void*)0,0);
		cout << endl << buff << " In client" << endl;
	}
//	shmdt(buff);
}
