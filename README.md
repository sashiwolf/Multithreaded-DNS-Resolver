# Multithreaded DNS Resolver

A multithreaded DNS resolver written in C that uses pthreads, mutexes, and semaphores to process hostname lookups concurrently through a shared circular buffer.

## Overview

This project demonstrates core operating systems and systems programming concepts including:

- Multithreading with pthreads
- Producer-consumer synchronization
- Mutex locks
- Semaphores
- Shared memory coordination
- Circular buffer implementation
- Concurrent hostname resolution

The resolver uses producer and consumer threads to efficiently process DNS lookup requests in parallel while safely sharing data between threads.

## Features

- Concurrent DNS hostname resolution
- Thread-safe shared array implementation
- Circular buffer for producer-consumer communication
- Synchronization using mutexes and semaphores
- Dynamic handling of shared resources
- Low-level systems programming in C

## Technologies Used

- C
- POSIX Threads (pthreads)
- Semaphores
- Mutexes
- Linux/Unix system APIs

## What I Learned

Through this project, I gained experience working with:

- Thread synchronization and race condition prevention
- Concurrent programming patterns
- Producer-consumer architecture
- Memory sharing between threads
- Low-level systems development in Linux
- Debugging multithreaded applications

## Example Concepts Implemented

- `pthread_mutex_lock()` and `pthread_mutex_unlock()`
- `sem_wait()` and `sem_post()`
- Circular queue management
- Safe concurrent access to shared data structures

## Build and Run

Compile the project:

```bash
gcc *.c -o resolver -lpthread
```

Run the executable:

```bash
./resolver
```

## Author

Sashi Wolf
