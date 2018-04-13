/*
 * Banker's Algorithm for SOFE 3950U / CSCI 3020U: Operating Systems
 *
 * Copyright (C) 2015, <GROUP MEMBERS>
 * All rights reserved.
 * 
 */
#ifndef BANKER_H_
#define BANKER_H_

#include <stdbool.h>

// Add any additional data structures or functions you want here
// perhaps make a clean bank structure instead of several arrays...

//thread argument struct to be passed to the thread in the form of a null pointer
	typedef struct threadArgument{
		int customerNumber;
		int resourceArray[];
	}threadArg;

// Request resources, returns true if successful
//int n_customer, int request[]
extern bool request_res(void * a);

// Release resources, returns true if successful
extern bool release_res(void * a);

#endif /* BANKER_H_ */