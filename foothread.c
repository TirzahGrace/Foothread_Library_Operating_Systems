#define _GNU_SOURCE
#include "foothread.h"
#include <unistd.h>
#include <sched.h>

// Thread start routine
int foothread_start_routine(void *arg) {
    int (*start_routine)(void *) = arg;
    start_routine(NULL); // Call the provided start routine
    foothread_exit();    // Terminate the thread
    return 0;
}

// Create a new foothread
void foothread_create(foothread_t *thread, foothread_attr_t *attr, int (*start_routine)(void *), void *arg) {
    // Reserve space for the new thread's stack
    char *stack = (char *)malloc(attr->stack_size);
    if (stack == NULL) {
        perror("Failed to allocate stack for thread");
        exit(EXIT_FAILURE);
    }

    // Call clone with appropriate arguments to create the new thread
    thread->tid = clone((int (*)(void *))start_routine, stack + attr->stack_size, CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM, arg);
    if (thread->tid == -1) {
        perror("Failed to create thread");
        exit(EXIT_FAILURE);
    }
}

// Set join type attribute
void foothread_attr_setjointype(foothread_attr_t *attr, int join_type) {
    attr->join_type = join_type;
}

// Set stack size attribute
void foothread_attr_setstacksize(foothread_attr_t *attr, size_t stack_size) {
    attr->stack_size = stack_size;
}

// Exit the current foothread
void foothread_exit() {
    exit(EXIT_SUCCESS);
}

// Initialize a mutex
void foothread_mutex_init(foothread_mutex_t *mutex) {
    sem_init(&mutex->sem, 0, 1); // Initialize semaphore with value 1 for mutex
}

// Lock a mutex
void foothread_mutex_lock(foothread_mutex_t *mutex) {
    sem_wait(&mutex->sem); // Wait until the semaphore is available
}

// Unlock a mutex
void foothread_mutex_unlock(foothread_mutex_t *mutex) {
    sem_post(&mutex->sem); // Release the semaphore
}

// Destroy a mutex
void foothread_mutex_destroy(foothread_mutex_t *mutex) {
    sem_destroy(&mutex->sem);
}

// Initialize a barrier
void foothread_barrier_init(foothread_barrier_t *barrier, int count) {
    barrier->count = count;
    barrier->arrived_count = 0;
    foothread_mutex_init(&barrier->mutex);
    sem_init(&barrier->sem, 0, 0); // Initialize semaphore with 0
}

// Wait on a barrier
void foothread_barrier_wait(foothread_barrier_t *barrier) {
    foothread_mutex_lock(&barrier->mutex);
    barrier->arrived_count++; // Increment the count of arrived threads
    if (barrier->arrived_count == barrier->count) {
        // If all threads have arrived, release the semaphore for all
        for (int i = 0; i < barrier->count - 1; ++i) {
            sem_post(&barrier->sem); // Release semaphore count times except for the last thread
        }
        barrier->arrived_count = 0; // Reset the count for the next barrier
        foothread_mutex_unlock(&barrier->mutex);
    } else {
        foothread_mutex_unlock(&barrier->mutex);
        sem_wait(&barrier->sem); // Block the thread until all threads have arrived
    }
}

// Destroy a barrier
void foothread_barrier_destroy(foothread_barrier_t *barrier) {
    sem_destroy(&barrier->sem);
    foothread_mutex_destroy(&barrier->mutex);
}
