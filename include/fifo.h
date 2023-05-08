#pragma once

#include <semaphore.h>

// FIFO
typedef struct {
    int* buf;  // Buffer that holds the data
    int head;  // Index of the first element
    int tail;  // Index of the next free element
    int size;  // Size of the buffer
} fifo_t;

// Push a new element into the queue
// [!] Does NOT check for overflow
void fifo_push(fifo_t* f, int data);

// Remove element from the queue and return its value
// [!] Does NOT check for underflow
int fifo_pop(fifo_t* f);

// Returns number of elements in the queue
int fifo_count(fifo_t* f);

// Returns value of the first element in the queue
int fifo_top(fifo_t* f);

// Counts even numbers in the queue
int fifo_count_even(fifo_t* f);

// Counts odd numbers in the queue
int fifo_count_odd(fifo_t* f);