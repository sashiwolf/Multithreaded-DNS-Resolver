#include "array.h"

int array_init(array *s) {
    // Set all the variables
    s->head = 0;
    s->tail = 0;
    s->count = 0;

    // Initialize the mutex and the semaphores
    if (pthread_mutex_init(&s->mutex, NULL) != 0) { // Null means use default mutex settings
        return -1;
    }

    if (sem_init(&s->empty, 0, ARRAY_SIZE) != 0) { // 0 means use it for threads in this program
        pthread_mutex_destroy(&s->mutex);
        return -1;
    }

    if (sem_init(&s->full, 0, 0) != 0) {
        sem_destroy(&s->empty);
        pthread_mutex_destroy(&s->mutex);
        return -1;
    }

    return 0;
}

int array_put(array *s, char *hostname) {
    // Wait until there's space in the array and lock the array
    sem_wait(&s->empty);
    pthread_mutex_lock(&s->mutex);

    // Insert the new string into the array
    s->data[s->tail] = hostname; 
    s->tail = (s->tail + 1) % ARRAY_SIZE;
    s->count++;

    // Unlock the array
    pthread_mutex_unlock(&s->mutex);

    // Increment the full count
    sem_post(&s->full);

    return 0;
}

int array_get(array *s, char **hostname) {
    // Wait until array has at least one slot full then lock the array
    sem_wait(&s->full);
    pthread_mutex_lock(&s->mutex);

    *hostname = s->data[s->head];
    s->head = (s->head + 1) % ARRAY_SIZE;
    s->count--;

    // Unlock the array
    pthread_mutex_unlock(&s->mutex);

    // Increment the empty count
    sem_post(&s->empty);

    return 0;
}

void array_free(array *s) {
    // Destroy the lock
    pthread_mutex_destroy(&s->mutex);

    // Destory the semaphores
    sem_destroy(&s->empty);
    sem_destroy(&s->full);
}