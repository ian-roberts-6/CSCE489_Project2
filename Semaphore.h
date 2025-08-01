#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore 
{
public:

	Semaphore(int count);
	~Semaphore();

	void wait();
	void signal();

private:
	int s_value;
	pthread_mutex_t semaphore_lock;
	pthread_cond_t semaphore_incremented;
};

#endif
