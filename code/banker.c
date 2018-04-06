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


// Define functions declared in banker.h here
bool request_res(int n_customer, int request[]){
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
            return retVal;

        }
        else{
            printf("request at i > need at i\n");
            exit(1);
        }
    }

}


// Release resources, returns true if successful
bool release_res(int n_customer, int release[]){
    bool done = false;
    int requested,remaining;
	int checker = 0;
	for(int i =0;i<NUM_RESOURCES;i++){
		if (release[i]>available[i]){
			checker++;
		}
	}
	if (checker>0){
		printf("not enough resources\n");
		return false;
	}
	for(int i =0;i<NUM_RESOURCES;i++){

	}
	return true;

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
	// ==================== YOUR CODE HERE ==================== //
	//seed RNG
	if(argc == 1){
		printf("no arguments given as seed so default of current time was given\n");
		srand(time(NULL));
	}
	else if(argc>=3){
		printf("only one argument please!\n");
	}
	else{
		//SEEDS rng using user input if given
		srand(abs(atoi(argv[1])));
	}


	// Read in arguments from CLI, NUM_RESOURCES is the number of arguments   

	// Allocate the available resources

	for(int i = 0;i<NUM_RESOURCES;i++){
		available[i]=MAX_RES*NUM_CUSTOMERS;
		work[i]=available[i];
	}
	//initialize maximum so each customer is allocated the same amount
	for(int i =0;i<NUM_CUSTOMERS;i++){
		for(int j=0;j<NUM_RESOURCES;j++){
			maximum[i][j]=MAX_RES;
		}
	}

	//initialize finish to all false
	for(int i =0;i<NUM_CUSTOMERS;i++){
		finish[i]=false;
	}

    
	// Initialize the pthreads, locks, mutexes, etc.
	pthread_t banker,client;

	sem_t customerSemaphore;
	sem_init(&customerSemaphore,0,NUM_CUSTOMERS); //semaphore so no more than the max amount of customers are making a request at one time
	pthread_mutex_init(&mutexlock,NULL);


	typedef struct threadArgument{
		int customerNumber;
		int resourceArray[];
	}threadArg;

    threadArg clientArg,bankerArg;

	// Run the threads and continually loop
	while(1){
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
            
			pthread_create(&client,NULL,(void *)request_res,(void*)&clientArg);
    		pthread_create(&banker,NULL,(void *)release_res,(void*)&bankerArg);

			// The threads will request and then release random numbers of resources
		}
	}
	// If your program hangs you may have a deadlock, otherwise you *may* have
	// implemented the banker's algorithm correctly

	// If you are having issues try and limit the number of threads (NUM_CUSTOMERS)
	// to just 2 and focus on getting the multithreading working for just two threads

	pthread_mutex_destroy(&mutexlock);
	sem_destroy(&customerSemaphore);
	return EXIT_SUCCESS;
}
