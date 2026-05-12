#include <stddef.h>  // defines size_t
#include <time.h>   // for clock_gettime
#include "multi-lookup.h"
#include "array.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

array sharedArray;
pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

static int resolve_ipv4_address(const char *hostname, char *ipv4, size_t ipv4_len) {
  struct addrinfo hints;
  struct addrinfo *results = NULL;
  struct addrinfo *current = NULL;
  int status = 0;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  status = getaddrinfo(hostname, NULL, &hints, &results);
  if (status != 0) {
    return -1;
  }

  for (current = results; current != NULL; current = current->ai_next) {
    struct sockaddr_in *address = (struct sockaddr_in *) current->ai_addr;
    if (inet_ntop(AF_INET, &(address->sin_addr), ipv4, ipv4_len) != NULL) {
      freeaddrinfo(results);
      return 0;
    }
  }

  freeaddrinfo(results);
  return -1;
}

void *requester(void *arg) {
    char *fileName = (char *)arg; // Turns arg back into char pointer
    FILE *file = fopen(fileName, "r");

    if (!file) {
        fprintf(stderr, "Invalid file %s\n", fileName);
        return NULL; // Must return void* in pthreads
    }

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        // Change the newline char to null byte
        line[strcspn(line, "\n")] = '\0';

        // Make a copy of the string for the shared array
        char *hostname = strdup(line);
        if (!hostname) {
            fprintf(stderr, "Memory allocation failed for hostname\n");
            continue; // skip this hostname if strdup fails
        }

        array_put(&sharedArray, hostname);
    }

    fclose(file);
    return NULL; // pthread function must return void*
}

void *resolver(void *arg) {
    FILE *logFile = (FILE *)arg;
    char *hostname = NULL;
    char ipv4[INET_ADDRSTRLEN];

    while (1) {
        // Get a hostname from the shared array
        if (array_get(&sharedArray, &hostname) != 0) {
            // If array_get fails (shouldn't normally happen), just continue
            continue;
        }

        if (hostname == NULL) {
            // NULL is our sentinel to signal no more hostnames
            break;
        }

        // Resolve hostname
        int status = resolve_ipv4_address(hostname, ipv4, sizeof(ipv4));

        // Lock the file while writing
        pthread_mutex_lock(&logMutex);
        if (status == 0) {
            fprintf(logFile, "%s, %s\n", hostname, ipv4);
        } else {
            fprintf(logFile, "%s, NOT_RESOLVED\n", hostname);
        }
        pthread_mutex_unlock(&logMutex);

        free(hostname); // Need to use free here because each hostname was created with strdup, so every hostname has its own space in memory
    }

    return NULL;
}

int main(int argc, char *argv[]) {
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  if (argc < 3) { 
    // Not enough arguments
    printf("Usage: %s <log_file> [ <input_files> ... ]\n", argv[0]);
    return -1;
  }

  // argv[1] is the log file name
  char *logFileName = argv[1];

  FILE *logFile = fopen(logFileName, "w");

  if (!logFile) {
    fprintf(stderr, "Invalid file %s\n", logFileName);
    return -1;
  }

  if (array_init(&sharedArray) != 0) {
    printf("array initialization failed\n");
    fclose(logFile);
    return -1;
  }

  // Initialize the requesters based on number of input files
  int numRequesterThreads = argc - 2;
  pthread_t requesterThreads[numRequesterThreads];

  // Get the requesters started
  for (int i = 0; i < numRequesterThreads; i++) {
    char *file = argv[i + 2];
    pthread_create(&requesterThreads[i], NULL, requester, (void *)file);
  }

  // Initialize the resolvers
  int numResolverThreads = 20;
  pthread_t resolverThreads[numResolverThreads];

  // Get the resolvers started
  for (int i = 0; i < numResolverThreads; i++) {
    pthread_create(&resolverThreads[i], NULL, resolver, (void *)logFile);
  }

  // Wait for requesters to finish
  for (int i = 0; i < numRequesterThreads; i++) {
      pthread_join(requesterThreads[i], NULL);
  }

  // Send NULLs to resolvers to signal completion
  for (int i = 0; i < numResolverThreads; i++) {
      array_put(&sharedArray, NULL);
  }

  // Wait for resolvers to finish
  for (int i = 0; i < numResolverThreads; i++) {
      pthread_join(resolverThreads[i], NULL);
  }

  fclose(logFile);
  array_free(&sharedArray);

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; // The 1e9 means 10^9 which is a billion and lets us convert from nano secs to seconds

  printf("./multi-lookup: total time is %.6f seconds\n", elapsed);

  return 0;
}