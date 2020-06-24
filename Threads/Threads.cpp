/*
 * Threads.cpp
 *
 *  Created on: 19-Jun-2020
 *      Author: c_ramjee
 */

#include "Threads.h"
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

using namespace std;

enum state {
	CONSUMED,
	PRODUCED,
} stateVar;

int count = 0;
sem_t mutex;
sem_t mutex_p;
pthread_mutex_t mlock;


void *  threadMClient(void *arg) {
		int count = 0;
		cout << "\n Enter Thread Client "<< pthread_self() << endl;
		while (true) {
			pthread_mutex_lock(&mlock);
			cout << "Client lock acquired" << endl;
			sleep(1);
			pthread_mutex_unlock(&mlock);
		}
		return NULL;
}

void *  threadMServer(void *arg) {
		int count = 0;
		cout << "\n Enter Thread Server "<< pthread_self() << endl;
		while(true) {
			pthread_mutex_lock(&mlock);
			cout << "Server  lock acquired" << endl;
			sleep(1);
			pthread_mutex_unlock(&mlock);
		}

		return NULL;
}
void threadMutex(){
	pthread_t t1, t2, t3, t4;
	pthread_attr_t attr;
	int size;

	pthread_attr_init(&attr);


	pthread_attr_getstacksize(&attr, (size_t *)&size);
	cout << "\n Ramjee Stack Size default is  " << size <<endl;
	pthread_attr_setstacksize(&attr, 1024*512);
	pthread_attr_getstacksize(&attr, (size_t *)&size);
	cout << "\n Ramjee Stack Size is  " << size <<endl;

	int *retval1, *retval2, *retval3, *retval4;
	if (pthread_mutex_init(&mlock, NULL) < 0){
		cout << "\n Mutex init failed \n";
	}
	pthread_create(&t1, &attr, threadMServer, NULL);
	sleep(1);
	pthread_create(&t2, &attr, threadMServer, NULL);
	sleep(1);
	pthread_create(&t3, &attr, threadMClient, NULL);
	sleep(1);
	pthread_create(&t4, &attr, threadMClient, NULL);
	sleep(1);
    pthread_setname_np(t1, "TestThread1");
    pthread_setname_np(t2, "TestThread2");
    pthread_setname_np(t3, "TestThread3");
    pthread_setname_np(t4, "TestThread4");
	pthread_join(t1, (void **)&retval1);
	pthread_join(t2, (void **)&retval2);
	pthread_join(t3, (void **)&retval3);
	pthread_join(t4, (void **)&retval4);
	pthread_mutex_destroy(&mlock);
}

void *  threadClient(void *arg) {
		int semVal = -1;
		int count = 0;
		cout << "\n Enter Thread Client "<< pthread_self() << endl;

		while (true) {
			sem_wait(&mutex);
			if (stateVar == PRODUCED) {
				stateVar = CONSUMED;
				sem_getvalue(&mutex, &semVal);
				cout << "\n Consumed  " << count-- << " semaphore value = "<< semVal << " Thread ID = "<< pthread_self() <<"\n";
			}
			sleep(1);
			sem_post(&mutex);
		}
		return NULL;
}

void *  threadServer(void *arg) {
		int semVal = -1;
		int count = 0;
		cout << "\n Enter Thread Server "<< pthread_self() << endl;
		while(true) {
			sem_wait(&mutex);
			if (stateVar == CONSUMED) {
				stateVar = PRODUCED;
				sem_getvalue(&mutex, &semVal);
				cout << "\n Produced  " << count++ << "  semaphore value = "<< semVal << " Thread ID = "<< pthread_self() <<"\n";
			}
			sleep(1);
			sem_post(&mutex);
		}

		return NULL;
}


void threadSem(){
	pthread_t t1, t2, t3, t4;
	int *retval1, *retval2, *retval3, *retval4;
	sem_init(&mutex, 0, 4);
	pthread_create(&t1, NULL, threadServer, NULL);
	sleep(1);
	pthread_create(&t2, NULL, threadServer, NULL);
	sleep(1);
	pthread_create(&t3, NULL, threadClient, NULL);
	sleep(1);
	pthread_create(&t4, NULL, threadClient, NULL);
	sleep(1);
	pthread_join(t1, (void **)&retval1);
	pthread_join(t2, (void **)&retval2);
	pthread_join(t3, (void **)&retval3);
	pthread_join(t4, (void **)&retval4);
	sem_destroy(&mutex);
}


void processServer (){
	key_t key = ftok("ipcfile.txt", 99999);
	int handle = shmget(key, 1024, 0666 | IPC_CREAT);
	state *stateVar = (state *) shmat(handle,(void*)0,0);
	int semVal = -1;
	while(true) {
		sem_wait(&mutex_p);
		if (*stateVar == CONSUMED) {
			*stateVar = PRODUCED;
			sem_getvalue(&mutex_p, &semVal);
			cout << "\n Producer  " << *stateVar << "  semaphore value = "<< semVal <<"\n";
		}
		sem_post(&mutex_p);
	}
	shmdt(stateVar);
}

void processClient (){
	key_t key = ftok("ipcfile.txt", 99999);
	int handle = shmget(key, 1024, 0666 | IPC_CREAT);
	state *stateVar = (state *) shmat(handle,(void*)0,0);
	int semVal = -1;
	while(true) {
		sem_wait(&mutex_p);
		if (*stateVar == PRODUCED) {
			*stateVar = CONSUMED;
			sem_getvalue(&mutex_p, &semVal);
			cout << "\n Producer  " << *stateVar  << "  semaphore value = "<< semVal <<"\n";
		}
		sem_post(&mutex_p);
	}
	shmdt(stateVar);
}

void processSem(){
	pid_t pid = -1;
	sem_init(&mutex_p, 1, 4);

	if ( (pid = fork()) < 0) {
		cout << "Fork failed";
	} else if (pid > 0 ) {
		int chldStatus;
		cout << "Fork in parent process pid = " << pid << endl;
		processServer();
		cout << "Fork parent wait pid "<< endl;
		waitpid(pid, &chldStatus, 0);
		cout << "Fork parent wait done "<< endl;
	} else {
		cout << "Fork in child process pid = " << pid << endl;
		processClient();
	}
	sem_destroy(&mutex_p);
}

int main(int argc, char** argv){
//	processSem();
//	threadSem();
	argv[0]="TestProcessName";
	threadMutex();
}
