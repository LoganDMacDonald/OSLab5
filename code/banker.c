/*
 * Banker's Algorithm for SOFE 3950U / CSCI 3020U: Operating Systems
 *
 * Copyright (C) 2015, <GROUP MEMBERS>
 * All rights reserved.
 * 
 */

/*
 * customer_n wants to retrieve request[] resources eg. {1,0,3} in the cast of 3 possible resources

 3 diff resources
 5 diff customers
 the following example table is a visualization of the maximum, allocation, and need matricies
 A to B is resources,
 1-5 is customers
 A   B   C
 1
 2
 3
 4
 5

 therefore request[] and available[] are arrays of size 3 (eg {1,0,3})
 representing resources A,B,C

*/
#include<time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "banker.h"

// Put any other macros or constants here using #define
// May be any values >= 0
#define NUM_CUSTOMERS 5
#define NUM_RESOURCES 3

#define MAX_RES 10
#define INIT_RES 20


// Put global environment variables here
// Available amount of each resource
int available[NUM_RESOURCES];
int work[NUM_RESOURCES];

// Maximum demand of each customer
int maximum[NUM_CUSTOMERS][NUM_RESOURCES];

// Amount currently allocated to each customer
int allocation[NUM_CUSTOMERS][NUM_RESOURCES];

// Remaining need of each customer
int need[NUM_CUSTOMERS][NUM_RESOURCES];

bool finish[NUM_CUSTOMERS];

pthread_mutex_t mutexlock;
sem_t customerSemaphore;

int stateChecker();

// Define functions declared in banker.h here
bool request_res(void * a){
	int request[NUM_RESOURCES];
	threadArg *ta= (threadArg*)(a);
	int n_customer=ta->customerNumber;
	for(int i =0;i<NUM_RESOURCES;i++){
		request[i]=ta->resourceArray[i];
	}
	
	printf("customer #%i requesting following resources\nRes1: %i, res2: %i, res3: %i\n",n_customer,request[0],request[1],request[2]);	
	bool retVal=true;
	for(int i =0;i<NUM_RESOURCES;i++){
		
		if (request[i]<=need[n_customer][i]){
			while(request[i]<available[i]){
				//wait until enough resources are available before allocating
				//POTENTIAL DEADLOCK LOCATION!!! waiting indefinitely until a resource is available
			}
			//when enough resource i is available
			pthread_mutex_lock(&mutexlock);
			available[i]-=request[i];
			allocation[n_customer][i]+=request[i];
			need[n_customer][i]-=request[i];

			//check safe, 
			if(stateChecker()){

			}
			else{ //if not restore old values
				available[i]+=request[i];
				allocation[n_customer][i]-=request[i];
				need[n_customer][i]+=request[i];
				//it says to wait for request but im just going to return false as to DECLINE
				retVal=false;
			}
			pthread_mutex_unlock(&mutexlock);


		}
		else{
			printf("ERROR: request at i is greater than the need at i\n");
			return false;
		}
	}
	if(retVal == false){
		printf("DENIED!\n");
	}
	else{
		printf("APPROVED!\n");
	}
	return retVal;
}
//int n_customer, int release[]

// Release resources, returns true if successful
bool release_res(void * a){
	int release[NUM_RESOURCES];

	threadArg *ta= (threadArg*)(a);
	int n_customer=ta->customerNumber;
	for(int i =0;i<NUM_RESOURCES;i++){
		release[i]=ta->resourceArray[i];
	}

	printf("customer #%i requesting following resources\nRes1: %i, res2: %i, res3: %i\n",n_customer,release[0],release[1],release[2]);	
	//actually convert void pointer argument struct to proper values

	// for each resource of n_customer
	// check if resultant resource is both less than allowed resource for that customer, 
	// and creates a safe state(statechecker)
	//if not safe state, revert back to previous state and return false
	bool retVal;
	for(int i = 0;i<NUM_RESOURCES;i++){
		if (allocation[n_customer][i]+release[i]>maximum[n_customer][i]){
			printf("resulting vector exceeds maximum\n");
			return false;
		}
		//actually release the resources now, then check for safe state
		for(int i =0;i<NUM_RESOURCES;i++){
			pthread_mutex_lock(&mutexlock);
			available[i]+=release[i];
			allocation[n_customer][i]-=release[i];
			need[n_customer][i]+=release[i];
			//check safe, 
			if(stateChecker()){
				retVal = true;
			}
			else{ //if not restore old values

				//it says to wait for request but im just going to return false as to DECLINE

				available[i]-=release[i];
				allocation[n_customer][i]+=release[i];
				need[n_customer][i]-=release[i];

				retVal=false;
			}
			pthread_mutex_unlock(&mutexlock);


		}


	}
	if(retVal == false){
		printf("DENIED!\n");
	}
	else{
		printf("APPROVED!\n");
	}
	return retVal;
}

bool finishChecker(){
	bool check = true;
	for(int i = 0;i<NUM_CUSTOMERS;i++){
		if(finish[i] == false){
			check = false;
			break;
		}
	}
	return check;
}

int stateChecker(){
	int needSum=0,allocationSum =0;
	for(int i = 0;i<NUM_CUSTOMERS;i++){
		if(finish[i]==false){
			for(int j = 0;j<NUM_RESOURCES;j++){
				needSum+=need[i][j];
				allocationSum+=allocation[i][j];
			}
			if(needSum<=work[i]){  
				work[i]+=allocationSum;
				finish[i]=true;
			}
		}
	}
	if(finishChecker()){
		return 1;
	}
	else{return 0;}
}




int main(int argc, char *argv[])
{
	int s;
	pthread_attr_t attr;
	s = pthread_attr_init(&attr);
	// ==================== YOUR CODE HERE ==================== //
	//seed RNG
	srand(time(NULL));
	printf("RNG set!\n");
	if(argc <=3){
		printf("not enough arguments given\n");
		exit(0);
	}
	else if(argc>=5){
		printf("only 3 argument please!\n");
		exit(0);
	}
	available[0]=atoi(argv[1]);
	available[1]=atoi(argv[2]);
	available[2]=atoi(argv[3]);
	
	// Read in arguments from CLI, NUM_RESOURCES is the number of arguments   
	// Allocate the available resources

	printf("available and work vectors initialized!\n");
	//initialize maximum so each customer is allocated the same amount
		//initialize finish to all false
	for(int i =0;i<NUM_CUSTOMERS;i++){
		finish[i]=false;
		for(int j=0;j<NUM_RESOURCES;j++){
			maximum[i][j]=MAX_RES;
		}
	}
	printf("maximum vector and finish vector initialized!\n");

	
	// Initialize the pthreads, locks, mutexes, etc.
	pthread_t banker[16],client[16];


	
	sem_init(&customerSemaphore,0,NUM_CUSTOMERS); //semaphore so no more than the max amount of customers are making a request at one time
	pthread_mutex_init(&mutexlock,NULL);
	printf("semaphore, thread, and mutex types declared!\n");



	threadArg clientArg,bankerArg;
	printf("thread argument data type set for banker and client arguments!\n");
	// Run the threads and continually loop

	for (int counter =0;counter<16;counter++){
		for(int eachCustomer = 0; eachCustomer<NUM_CUSTOMERS;eachCustomer++){


			//generate random data for client to request
			clientArg.customerNumber = eachCustomer;
			for(int i = 0;i<NUM_RESOURCES;i++){
				clientArg.resourceArray[i] = rand()%10;
			}

			//generate random data for banker to release for each customer
			bankerArg.customerNumber = eachCustomer;
			for(int i = 0;i<NUM_RESOURCES;i++){
				bankerArg.resourceArray[i] = rand()%10;
			}

			pthread_create(&client[counter],&attr,(void*)request_res,(void*)&clientArg);
			pthread_create(&banker[counter],&attr,(void*)release_res,(void*)&bankerArg);

			// The threads will request and then release random numbers of resources
		}
	}

	for (int i =0;i<16;i++){
		pthread_join(client[i],NULL);
		pthread_join(banker[i],NULL);
	}
	// If your program hangs you may have a deadlock, otherwise you *may* have
	// implemented the banker's algorithm correctly

	// If you are having issues try and limit the number of threads (NUM_CUSTOMERS)
	// to just 2 and focus on getting the multithreading working for just two threads

	pthread_mutex_destroy(&mutexlock);
	sem_destroy(&customerSemaphore);
	return EXIT_SUCCESS;
}
