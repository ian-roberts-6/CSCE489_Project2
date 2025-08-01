#include <pthread.h>
#include "Semaphore.h"


/*************************************************************************************
 * Semaphore (constructor) - this should take count and place it into a local variable.
 *						Here you can do any other initialization you may need.
 *
 *    Params:  count - initialization count for the semaphore
 *
 *************************************************************************************/

Semaphore::Semaphore(int count) {
    s_value = count;
    pthread_mutex_init(&semaphore_lock, NULL);
    pthread_cond_init(&semaphore_incremented, NULL);
}


/*************************************************************************************
 * ~Semaphore (destructor) - called when the class is destroyed. Clean up any dynamic
 *						memory.
 *
 *************************************************************************************/

Semaphore::~Semaphore() {
    pthread_mutex_destroy(&semaphore_lock);
    pthread_cond_destroy(&semaphore_incremented);
}


/*************************************************************************************
 * wait - implement a standard wait Semaphore method here
 *
 *************************************************************************************/

void Semaphore::wait() {
    pthread_mutex_lock(&semaphore_lock);
    while (s_value <= 0) {
        pthread_cond_wait(&semaphore_incremented, &semaphore_lock);
    }
    s_value--;
    pthread_mutex_unlock(&semaphore_lock);
}


/*************************************************************************************
 * signal - implement a standard signal Semaphore method here
 *
 *************************************************************************************/

void Semaphore::signal() {
    pthread_mutex_lock(&semaphore_lock);
    s_value++;
    pthread_mutex_unlock(&semaphore_lock);
    pthread_cond_signal(&semaphore_incremented);
}


