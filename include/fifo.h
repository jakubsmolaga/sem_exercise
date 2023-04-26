#pragma once

#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* buf;  // Buffer that holds the data
    int head;  // Index of the first element
    int tail;  // Index of the next free element
    int size;  // Size of the buffer
} fifo_t;

void fifo_push(fifo_t* f, int data);
int fifo_pop(fifo_t* f);
int fifo_count_odd(fifo_t* f);
int fifo_count_even(fifo_t* f);
int fifo_count(fifo_t* f);
int fifo_top(fifo_t* f);
void fifo_print(fifo_t* f);
