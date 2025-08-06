/*************************************************************************************
 * babyyoda - used to test your semaphore implementation and can be a starting point for
 *			     your store front implementation
 *
 *************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "Semaphore.h"

// Semaphores that each thread will have access to as they are global in shared memory
Semaphore *empty_slots = NULL;
Semaphore *full_slots = NULL;

pthread_mutex_t buf_mutex;

int* ring_buffer;
int shelf_size;
int producer_index = 0;
int consumer_index = 0;
int consumed = 0;
static bool quitthreads = false;
// The current serial number (incremented)
unsigned int serialnum = 1;
int num_produce = 0;
int left_to_produce = 0;

/*************************************************************************************
 * producer_routine - this function is called when the producer thread is created.
 *
 *			Params: data - a void pointer that should point to an integer that indicates
 *							   the total number to be produced
 *
 *			Returns: always NULL
 *
 *************************************************************************************/

void *producer_routine(void *data) {

	time_t rand_seed;
	srand((unsigned int) time(&rand_seed));
	
	// We know the data pointer is an integer that indicates the producer's id
	int producer_id = *(unsigned int*) data;
	int current_serial_num = 0;
	free(data);
	bool placed = false;

	// Loop through the amount we're going to produce and place into the buffer
	while (left_to_produce > 0) {
		printf("Producer %u wants to put a Yoda into buffer...\n", producer_id);

		// Semaphore check to make sure there is an available slot
		empty_slots->wait();

		// Place item on the next shelf slot by first setting the mutex to protect our buffer vars
		pthread_mutex_lock(&buf_mutex);
		if (left_to_produce > 0) {
			ring_buffer[producer_index] = serialnum;
			current_serial_num = serialnum;
			serialnum++;
			left_to_produce--;
			producer_index = (producer_index + 1) % shelf_size;
			placed = true;
		} else {
			placed = false;
		}
		pthread_mutex_unlock(&buf_mutex);
		if (placed) {
			printf("   Producer %u put Yoda #%u on shelf.\n", producer_id, current_serial_num);
			// Semaphore signal that there are items available
			full_slots->signal();
		} else {
			empty_slots -> signal(); //Release the empty slot currently held. 
		}
		


		// random sleep but he makes them fast so 1/20 of a second
		usleep((useconds_t) (rand() % 200000));	
	}
	return NULL;
}


/*************************************************************************************
 * consumer_routine - this function is called when the consumer thread is created.
 *
 *       Params: data - a void pointer that should point to a boolean that indicates
 *                      the thread should exit. Doesn't work so don't worry about it
 *
 *       Returns: always NULL
 *
 *************************************************************************************/

void *consumer_routine(void *data) {

	unsigned int consumer_id = *(unsigned int*) data;
	free(data);
	int bought_serial_num = -1;


	while (!quitthreads) {
		printf("Consumer %u wants to buy a Yoda...\n", consumer_id);

		// Semaphore to see if there are any items to take
		full_slots->wait();
		// Take an item off the shelf
		pthread_mutex_lock(&buf_mutex);

		bought_serial_num = ring_buffer[consumer_index];
		ring_buffer[consumer_index] = 0;
		consumed++;
		consumer_index = (consumer_index + 1) % shelf_size;
	
		pthread_mutex_unlock(&buf_mutex);

		printf("   Consumer %u bought Yoda #%d.\n", consumer_id, bought_serial_num);


		// Consumers wait up to one second
		usleep((useconds_t) (rand() % 1000000));

		empty_slots->signal();
	}
	// printf("Consumer goes home.\n");

	return NULL;	
}


/*************************************************************************************
 * main - Standard C main function for our storefront. 
 *
 *		Expected params: pctest <num_consumers> <max_items>
 *				max_items - how many items will be produced before the shopkeeper closes
 *
 *************************************************************************************/

int main(int argv, const char *argc[]) {

	const unsigned int NUM_PRODUCERS = 2;

	// Get our argument parameters
	if (argv < 4) {
		printf("Invalid parameters. Format: %s <shelf_size> <num_consumers> <max_items>\n", argc[0]);
		exit(0);
	}

	// User input on the size of the shelf
	shelf_size = (int) strtol(argc[1], NULL, 10);

	// User input on the number of consumers
	const unsigned int NUM_CONSUMERS = (unsigned int) strtol(argc[2], NULL, 10);

	// User input on the size of the buffer
	num_produce = (unsigned int) strtol(argc[3], NULL, 10);
	left_to_produce = num_produce;

	printf("Producing %d today.\n", num_produce);

	// Initialize our buffer
	ring_buffer = (int*) malloc(shelf_size * sizeof(int));
	
	// Initialize our semaphores
	empty_slots = new Semaphore(shelf_size);
	full_slots = new Semaphore(0);

	pthread_mutex_init(&buf_mutex, NULL); // Initialize our buffer mutex

	pthread_t *producers = (pthread_t*) malloc(NUM_PRODUCERS*sizeof(pthread_t));
	pthread_t *consumers = (pthread_t*) malloc(NUM_CONSUMERS*sizeof(pthread_t));

	
	// Launch our producer threads
	for (unsigned int i = 0; i < NUM_PRODUCERS; i++) {
		int* id = (int*) malloc(1*sizeof(int));	//Note: this allocated memory is freed in the consumer function.
		*id = i;
		pthread_create(&producers[i], NULL, producer_routine, (void *) id);

	}

	// Launch our consumer threads
	for (unsigned int i = 0; i < NUM_CONSUMERS; i++) {
		int* id = (int*) malloc(1*sizeof(int));	//Note: this allocated memory is freed in the consumer function.
		*id = i;
		pthread_create(&(consumers[i]), NULL, consumer_routine, id); 
	}

	// Wait for our producer threads to finish up
	for (unsigned int i = 0; i < NUM_PRODUCERS; i++) {
		pthread_join(producers[i], NULL);
	}

	printf("The manufacturer has completed his work for the day.\n");

	printf("Waiting for consumer to buy up the rest.\n");
	// Give the consumers a second to finish snatching up items
	while (consumed < num_produce)
		sleep(1);
	quitthreads = true;

	// We are exiting, clean up
	delete empty_slots;
	delete full_slots;
	free(consumers);
	free(producers);
	free(ring_buffer);

	printf("Producer/Consumer simulation complete!\n");

}
