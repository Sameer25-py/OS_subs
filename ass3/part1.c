#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "part1.h"
#include "main.h"
#include <stdlib.h>

/*
	- DO NOT USE SLEEP FUNCTION
	- Define every helper function in .h file
	- Use Semaphores for synchronization purposes
 */


/**
* Declare semaphores here so that they are available to all functions.
*/
// sem_t* example_semaphore;
const int MAX_NUM_FLOORS = 20;
sem_t* floor_sema_in;
sem_t* floor_sema_out;
sem_t* mutex;
int current_floor;
int max_people;
int passengers;
int waiting_people;


/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here. Remember this is C and uses Malloc for memory allocation.
 *
 * numFloors: Total number of floors elevator can go to. numFloors will be smaller or equal to MAX_NUM_FLOORS
 * maxNumPeople: The maximum capacity of the elevator
 *
 */
void initializeP1(int numFloors, int maxNumPeople){
	//example_semaphore = (sem_t*) malloc(sizeof(sem_t));
	//allocating semaphores
	floor_sema_in =  (sem_t*)(malloc(sizeof(sem_t)*MAX_NUM_FLOORS));
	floor_sema_out = (sem_t*)(malloc(sizeof(sem_t)*MAX_NUM_FLOORS));
	mutex = (sem_t*)(malloc(sizeof(sem_t)));
	
	//initializing vars
	max_people = maxNumPeople;
	passengers= 0;
	waiting_people = 0;
	current_floor = 0;

	//initializing semaphores;
	sem_init(mutex,0,1);
	for(int i=0;i<MAX_NUM_FLOORS;i++){sem_init(&floor_sema_in[i],0,0);sem_init(&floor_sema_out[i],0,0);}

	return;
}



/**
 * Every passenger will call this function when 
 * he/she wants to take the elevator. (Already
 * called in main.c)
 * 
 * This function should print info "id from to" without quotes,
 * where:
 * 	id = id of the user (would be 0 for the first user)
 * 	from = source floor (from where the passenger is taking the elevator)
 * 	to = destination floor (floor where the passenger is going)
 * 
 * info of a user x_1 getting off the elevator before a user x_2
 * should be printed before.
 * 
 * Suppose a user 1 from floor 1 wants to go to floor 4 and
 * a user 2 from floor 2 wants to go to floor 3 then the final print statements
 * will be 
 * 2 2 3
 * 1 1 4
 *
 */

void* start_simul(){
	int dir = 1; //up
	while(1){
			sem_wait(mutex);
			if(passengers==0 && waiting_people == 0)break;
			
			if(current_floor == 20)dir=0; //down
			else if(current_floor == 0)dir=1; //up
			
			if(passengers > 0)sem_post(&floor_sema_out[current_floor]); //people in lift can go out if to == current_floor
			if(waiting_people > 0 && passengers < max_people)sem_post(&floor_sema_in[current_floor]);//people on the floor can enter the lift if from == current_floor

			if(dir==1)current_floor++;
			else current_floor--;
			
			sem_post(mutex);
	}
	return;
}
void* goingFromToP1(void *arg){

	struct argument* args = (struct argument*) arg;

	int id = args->id;
	int from = args->from;
	int to = args->to;


	if((from < 0 || to > MAX_NUM_FLOORS) && (to < 0 || to > MAX_NUM_FLOORS)){
		printf("FLOOR VALUE OUT OF BOUNDS");
		return;
	}
	sem_wait(mutex);
	waiting_people+=1;    
	sem_post(mutex);
	
	//waiting for elevator to come
	sem_wait(&floor_sema_in[from]);

		sem_wait(mutex);
		waiting_people--;
		passengers++;
		sem_post(mutex);

	sem_wait(&floor_sema_out[to]); //wait until destination floor
	// FILE* fptr = fopen("program.txt","a");
	// fprintf(fptr,"%d %d\n",waiting_people,passengers);
	// fclose(fptr);
	sem_wait(mutex);
	printf("%d %d %d\n",id,from,to);
	passengers--;
	sem_post(mutex);

	return NULL;
}

/*If you see the main file, you will get to 
know that this function is called after setting every
passenger.

So use this function for starting your elevator. In 
this way, you will be sure that all passengers are already
waiting for the elevator.
*/
void startP1(){
	sleep(1);
	pthread_t t;
	pthread_create(&t,NULL,start_simul,NULL);
}