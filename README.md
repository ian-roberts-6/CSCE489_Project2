# AFIT-CSCE489-PROJ2

## Development Notes
This was my first project in C++, but the object-oriented features weren’t too difficult to manage. Implementing the wait() function without spinlocking required the pthread_cond_wait() function, but thankfully the docs  for all the pthread functions were extremely helpful. 

One of the biggest questions in this project is which item a consumer will take from the buffer. That is, given a buffer that has some items in it and a consumer that is ready to consume one, which item will they take? I went with a ring buffer solution, with the producer having an index to write to and right behind it the consumers having an index they take from. If the indexes increase to larger than the buffer size, they wrap around to the start of the buffer. The semaphores are used to ensure that the producer does not overfill the buffer or that the consumers try to read from the empty buffer. 

The one problem I could not solve is the consumer threads exiting. Put simply, if a consumer thread is waiting at the buffer semaphore after the last Baby Yoda is removed from the buffer, it will be stuck there until the program terminates. For this project, I determined that this isn’t really an issue, as the consumer threads still complete their work and the program doesn’t need to wait for them to do anything more to exit successfully. Perhaps there is a way to ensure the consumers break out of the loop after the buffer is finally emptied, but I didn’t find that, and for this project I didn’t need to. 

## Required Questions

### Assuming the shopkeeper hires another worker, what changes would you need to make to turn this into a multi-producer, multi-consumer solution? 

To implement the multi-producer multi-consumer solution, we would need some way for the producers to track the correct serial number and number left to produce between each other. This is not implemented in the base simulation because it doesn’t have to be, but to add this, we need to move the `num_produce`, `left_to_produce`, and `serialnum` variable up to a static scope so all threads can access them. These would only be updated in the critical section of the buffer. Lastly, to prevent the producer from producing more than their allotted amount, there needs to be an if statement inside the critical section for the buffer variables to check. This would look like the following:

```
int* buffer;
int producer_index = 0;
static bool quitthreads = false;
unsigned int serialnum = 1;
int num_produce = NUM_TO_PRODUCE;
int left_to_produce = num_produce;




void *producer_routine(void *data) {
	int producer_id = *(unsigned int*) data;
	int current_serial_num = 0;
	bool placed = false;
	while (left_to_produce > 0) {
		empty_slots->wait();
		pthread_mutex_lock(&buf_mutex);
		if (left_to_produce > 0) {
			buffer[producer_index] = serialnum;
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
			full_slots->signal();
		} else {
			empty_slots -> signal(); 
		}
	}
	return NULL;
}
```

And in the main function, it’s just a matter of creating multiple producer threads the same way the consumer is implemented. Unlike the consumer, however, this program will wait for all the producers to finish. 

To make sure this idea works, I wrote the code and ran it! You can find the implementation on the git repository under the `multi-producer` branch. It creates two producers alongside the user-specified number of consumers. The main takeaway is that more producers means a few more shared variables, but the same critical sections and same overall code structure. 
