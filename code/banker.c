/*
 * Banker's Algorithm for SOFE 3950U / CSCI 3020U: Operating Systems
 *
 * Copyright (C) 2015, 
 * Logan MacDonald 		
 * Christian MacLeod	
 * Riley Stephens		
 * 
 * All rights reserved.
 * 
 */

/*
 * customer_n wants to retrieve request[] resources eg. {1,0,3} in the cast of 3 possible resources
 *
 3 diff resources
 5 diff customers
 the following example table is a visualization of the maximum, allocation, and need matricies
 A to B is resources,
 1-5 is customers
	A	B  	C
 1	x	x	x
 2	x	x	x	
 3	x	x	x	
 4	x	x	x
 5	x	x	x

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

pthread_mutex_t mutexlock, ml1;
sem_t customerSemaphore1,customerSemaphore2;

// Initialize the pthreads, locks, mutexes, etc.
pthread_t banker,client;

int stateChecker(); //declare statechecker to be used in request and release function


bool request_res(void * a){
	//convert void pointer argument into usable values
	int request[NUM_RESOURCES];
	threadArg *ta= (threadArg*)(a);
	int n_customer=ta->customerNumber;
	for(int i =0;i<NUM_RESOURCES;i++){
		request[i]=ta->resourceArray[i];
		if (request[i]<0 || request[i]>10){ //if the number is not within expected range force it into expected range
			request[i]= abs(request[i])%10;
		}
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

			//check if state is safe
			if(stateChecker()){

			}
			else{ //if not safe restore old values
				available[i]+=request[i];
				allocation[n_customer][i]-=request[i];
				need[n_customer][i]+=request[i];
				//it says to wait for request but im just going to return false as to DECLINE
				retVal=false;
			}
			pthread_mutex_unlock(&mutexlock);


		}
		else{
			printf("DENIED: request at %i is greater than the need at %i\n",i,i);
			retVal = false;
			break;
		}
	}
	if(retVal == false){
		printf("DENIED!\n");
	}
	else{
		printf("APPROVED!\n");
	}
	pthread_mutex_unlock(&ml1);
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
		if (release[i]<0 || release[i]>10){
			release[i]= abs(release[i])%10;
		}
	}

	printf("bank requesting release of following resources for customer %i\nRes1: %i, res2: %i, res3: %i\n",n_customer,release[0],release[1],release[2]);	
	//actually convert void pointer argument struct to proper values

	// for each resource of n_customer
	// check if resultant resource is both less than allowed resource for that customer, 
	// and creates a safe state(statechecker)
	//if not safe state, revert back to previous state and return false
	bool retVal;
	for(int i = 0;i<NUM_RESOURCES;i++){
		if (allocation[n_customer][i]+release[i]>maximum[n_customer][i]){
			printf("resulting vector exceeds maximum\n");
			return retVal = false;
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

//returns true if any true values in finish array otherwise returns false
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

// utilizes banker algorithm to check if current state is safe
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





	sem_init(&customerSemaphore1,0,1);
	sem_init(&customerSemaphore2,0,1); //semaphore so no more than the max amount of customers are making a request at one time
	pthread_mutex_init(&mutexlock,NULL);	//mutex for data critical section
	pthread_mutex_init(&ml1,NULL);	//mutex for client thread
	printf("semaphore, thread, and mutex types declared!\n");

	// declare the thread argument for both the banker and the client
	threadArg clientArg,bankerArg;
	printf("thread argument data type set for banker and client arguments!\n");
	// Run the threads in a infinite loop
	while(true){
		for(int eachCustomer = 0; eachCustomer<NUM_CUSTOMERS;eachCustomer++){


			//generate random data for client to request
			clientArg.customerNumber = eachCustomer;
			for(int i = 0;i<NUM_RESOURCES;i++){
				clientArg.resourceArray[i] = abs(rand())%10; //creates random number between 0 and 9 inclusive for the client to request
			}

			//generate random data for banker to release for each customer
			bankerArg.customerNumber = eachCustomer;
			for(int i = 0;i<NUM_RESOURCES;i++){
				bankerArg.resourceArray[i] = abs(rand())%6; //creates a random number between 0 and 5 inclusive
			}

			pthread_create(&banker,NULL,(void*)release_res,(void*)&bankerArg);//start banker's thread
			if(pthread_mutex_trylock(&ml1)){
				// this will make sure only one client request thread is running at one time
				pthread_create(&client,NULL,(void*)request_res,(void*)&clientArg);
			}




			// The threads will request and then release random numbers of resources
		}
	}

	// If your program hangs you may have a deadlock, otherwise you *may* have
	// implemented the banker's algorithm correctly

	// If you are having issues try and limit the number of threads (NUM_CUSTOMERS)
	// to just 2 and focus on getting the multithreading working for just two threads

	pthread_mutex_destroy(&mutexlock);
	sem_destroy(&customerSemaphore1);
	sem_destroy(&customerSemaphore2);
	return EXIT_SUCCESS;
}
