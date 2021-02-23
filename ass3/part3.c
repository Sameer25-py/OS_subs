#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "part3.h"
#include "main.h"


/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */


/**
* Declare semaphores here so that they are available to all functions.
*/
// sem_t* example_semaphore;

/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here.
 */
void initializeP3() {
	// example_semaphore = (sem_t*) malloc(sizeof(sem_t));
}


/**
 * This is the function called for each car thread. You can check
 * how these functions are used by going over the test3 function
 * in main.c
 * If there is a car going from SOUTH to NORTH, from lane LEFT,
 * print 
 * SOUTH NORTH LEFT
 * Also, if two cars can simulateneously travel in the two lanes,
 * first print all the cars in the LEFT lane, followed by all the
 * cars in the right lane
 *
 * Input: *argu is of type struct argumentP3 defined in main.h
 */
void * goingFromToP3(void *argu){
	
	// Some code to help in understanding argu

}

void start_simul3(){
	
}

/**
 * startP3 is called once all cars have been initialized. The logic of the traffic signals
 * will go here
 */
void startP3(){
}