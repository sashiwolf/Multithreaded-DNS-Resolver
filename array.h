#pragma once
#include <pthread.h>
#include <semaphore.h>

#define ARRAY_SIZE 8

typedef struct {
    char *data[ARRAY_SIZE]; // This is the actual array
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    sem_t empty;
    sem_t full;
} array;


int array_init(array *s);                 // initialise the array
int array_put(array *s, char *hostname);  // place element into the array, block when full
int array_get(array *s, char **hostname); // remove element from the array, block when empty
void array_free(array *s);                // free the array's resources