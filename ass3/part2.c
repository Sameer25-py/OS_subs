#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include "part2.h"
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
const int INTER_ARRIVAL_TIME = 5;
const int NUM_TRAINS = 5;
sem_t* mutex;
int* waiting_queue; //people waiting in respected stations
int* passengers_queue; //passengers waiting in respected trains
int stations;   //total stations
sem_t* train_in; //signal to get in train
sem_t* train_out; //signal to get out train
int train_capacity;
int total_waiting; //combined people waiting on all stations
int total_travelling; //combined people waiting in all trains

/**
 * Do any initial setup work in this function. You might want to 
 * initialize your semaphores here. Remember this is C and uses Malloc for memory allocation.
 *
 * numStations: Total number of stations. Will be >= 5. Assume that initially
 * the first train is at station 1, the second at 2 and so on.
 * maxNumPeople: The maximum number of people in a train
 */
void initializeP2(int numStations, int maxNumPeople) {
	// example_semaphore = (sem_t*) malloc(sizeof(sem_t)); 
	stations = numStations;
	train_capacity = maxNumPeople;
	waiting_queue = (int*)(malloc(sizeof(int)*numStations));
	passengers_queue = (int*)(malloc(sizeof(int)*NUM_TRAINS));
	
	//initalize semaphores
	mutex = (sem_t*)(malloc(sizeof(sem_t)));
	train_in = (sem_t*)(malloc(sizeof(sem_t)*NUM_TRAINS));
	train_out = (sem_t*)(malloc(sizeof(sem_t)*numStations));

	sem_init(mutex,0,1);
	for(int i=0;i<NUM_TRAINS;i++){sem_init(&train_in[i],0,0);sem_init(&train_out[i],0,0);passengers_queue[i]= 0;}
	for(int i=0;i<numStations;i++)waiting_queue[i] = 0;


}

/**
	This function is called by each user.

 * Print in the following format:
 * If a user borads on train 0, from station 0 to station 1, and another boards
 * train 2 from station 2 to station 4, then the output will be
 * 0 0 1
 * 2 2 4
 */
void * goingFromToP2(void * user_data) {
	struct argument* args = (struct argument*) user_data;

	int id = args->id;
	int from = args->from;
	int to = args->to;

	if((from < 0 || to > stations) && (to < 0 || to > stations)){
		printf("STATION VALUE OUT OF BOUNDS");
		return;
	}
	sem_wait(mutex);
	total_waiting+=1;
	waiting_queue[from]+=1;
	sem_post(mutex);
	
	int train_number =0;
	while(1){
		if(wait(&train_in[train_number]))break; //race condition to board first available train
		train_number+=1;
		if(train_number == NUM_TRAINS)train_number = 0;
	}
	sem_wait(mutex);
	passengers_queue[train_number]+=1;
	total_travelling+=1;
	sem_post(mutex);

	sem_wait(&train_out[to]);

	sem_wait(mutex);
	total_travelling--;
	passengers_queue[train_number]--;
	printf("%d %d %d\n",id,from,to);
	sem_post(mutex);

	return NULL;

}

void start_simul2(int x){
	int current_station = 1;
	int elapsed_simulation_time = 0;
	while(1){
		sem_wait(mutex);
		if(total_travelling == 0 && total_waiting == 0){break;sem_post(mutex);}
		elapsed_simulation_time+=1;
		if(elapsed_simulation_time%INTER_ARRIVAL_TIME != 0){sem_post(mutex);continue;}
		//signal to get in
		if(waiting_queue[current_station] > 0 && passengers_queue[x] < train_capacity)sem_post(&train_in[x]);
		if(passengers_queue[x] > 0)sem_post(&train_out[current_station]);
		sem_post(mutex);
		current_station+=1;
		if(current_station == stations)current_station = 1;
	}
	return;
}


/* Use this function to start threads for your trains */
void * startP2(){
	sleep(1);
	for(int i=0;i<NUM_TRAINS;i++){
		pthread_t t;
		pthread_create(&t,NULL,start_simul2,i);
	}
	
}