#ifndef FOOTHREAD_H
#define FOOTHREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

// Maximum number of threads allowed
#define FOOTHREAD_THREADS_MAX 100

// Default stack size for threads
#define FOOTHREAD_DEFAULT_STACK_SIZE (2 * 1024 * 1024) // 2MB

// Join types
#define FOOTHREAD_JOINABLE 0
#define FOOTHREAD_DETACHED 1

// Thread attribute structure
typedef struct {
    int join_type;
    size_t stack_size;
} foothread_attr_t;

// Initialize thread attribute structure
#define FOOTHREAD_ATTR_INITIALIZER { FOOTHREAD_JOINABLE, FOOTHREAD_DEFAULT_STACK_SIZE }

// Thread data structure
typedef struct {
    int tid; // Unique identifier for the thread
    foothread_attr_t attr; // Thread attributes
    // Other fields for thread management, if needed
} foothread_t;

// Mutex data structure
typedef struct {
    sem_t sem; // Semaphore for mutual exclusion
} foothread_mutex_t;

// Barrier data structure
typedef struct {
    int count;               // Total number of threads expected at the barrier
    int arrived_count;       // Number of threads that have arrived at the barrier
    foothread_mutex_t mutex; // Mutex for synchronization
    sem_t sem;               // Semaphore for blocking at the barrier
} foothread_barrier_t;

// Function prototypes
void foothread_create(foothread_t *thread, foothread_attr_t *attr, int (*start_routine)(void *), void *arg);
void foothread_attr_setjointype(foothread_attr_t *attr, int join_type);
void foothread_attr_setstacksize(foothread_attr_t *attr, size_t stack_size);
void foothread_exit();
void foothread_mutex_init(foothread_mutex_t *mutex);
void foothread_mutex_lock(foothread_mutex_t *mutex);
void foothread_mutex_unlock(foothread_mutex_t *mutex);
void foothread_mutex_destroy(foothread_mutex_t *mutex);
void foothread_barrier_init(foothread_barrier_t *barrier, int count);
void foothread_barrier_wait(foothread_barrier_t *barrier);
void foothread_barrier_destroy(foothread_barrier_t *barrier);

#endif /* FOOTHREAD_H */
