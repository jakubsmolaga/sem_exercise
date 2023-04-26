# Semaphores exercise
This repository contains a solution to a simple semaphores exercise.

## Exercise
There is a FIFO queue for integers.
- Threads **A1** generate **even** numbers modulo **50** if the queue contains **less than 10 even numbers**.
- Threads **A2** generate **odd** numbers modulo **50** if the queue contains **less odd numbers than even numbers**.
- Threads **B1** consume **even** numbers from the queue if the queue contains **more than 3 numbers**.
- Threads **B2** consume **odd** numbers from the queue if the queue contains **more than 7 numbers**.

## Solution
### FIFO implementation
The FIFO structure is implemented as follows:
```C
typedef struct {
    int* buf;  // Buffer that holds the data
    int head;  // Index of the first element
    int tail;  // Index of the next free element
    int size;  // Size of the buffer
} fifo_t;
```
And the following functions are used to manipulate the FIFO:
```C
// Add a new element
void fifo_push(fifo_t* f, int data); 

// Remove the first element
int fifo_pop(fifo_t* f); 

// Count the number of odd elements
int fifo_count_odd(fifo_t* f); 

// Count the number of even elements
int fifo_count_even(fifo_t* f);

// Count the number of elements
int fifo_count(fifo_t* f); 

// Return the first element
int fifo_top(fifo_t* f);
```
The functions have no synchronization mechanisms, so they are not thread-safe.  
They also have no protection against buffer overflow or underflow.  
All of that functionality must be implemented by the user.
### Synchronization variables
```C
sem_t m_fifo;       // Mutex that locks access to the fifo
sem_t m_prod_even;  // Mutex that wakes up a waiting even producer
sem_t m_prod_odd;   // Mutex that wakes up a waiting odd producer
sem_t m_cons_even;  // Mutex that wakes up a waiting even consumer
sem_t m_cons_odd;   // Mutex that wakes up a waiting odd consumer

int wait_cnt_prod_even = 0;  // Number of waiting even producers
int wait_cnt_prod_odd = 0;   // Number of waiting odd producers
int wait_cnt_cons_even = 0;  // Number of waiting even consumers
int wait_cnt_cons_odd = 0;   // Number of waiting odd consumers
```
### Waiting routine
This routine waits for a condition to be met.  
It also keeps track of the number of waiting threads.  
Every condition must have a corresponding mutex.  
```C
void wait_for_condition(int (*cond)(), sem_t* m, int* wait_cnt) {
    // This function expects the fifo to be locked
    // Loop until the condition is met
    while (!cond()) {
        (*wait_cnt)++; // Add this thread to the waiting queue
        sem_post(&m_fifo); // Release the lock on the fifo
        sem_wait(m); // Wait for the condition to be met
        //
        // Between theese two locks the condition may have changed
        // That's why we need to check it again in a loop
        //
        sem_wait(&m_fifo); // Lock the fifo
        (*wait_cnt)--; // Remove this thread from the waiting queue
    }
};
```
### Update routine
This routine is called after every change to the fifo.  
It checks if any of the waiting threads can be woken up and wakes them up.  
```C
void update_semaphores() {
    // This function expects the fifo to be locked
    // Wake up waiting threads if possible
    if (wait_cnt_prod_even > 0 && can_prod_even())
        sem_post(&m_prod_even); // Wake up a waiting even producer
    if (wait_cnt_prod_odd > 0 && can_prod_odd())
        sem_post(&m_prod_odd); // Wake up a waiting odd producer
    if (wait_cnt_cons_even > 0 && can_cons_even())
        sem_post(&m_cons_even); // Wake up a waiting even consumer
    if (wait_cnt_cons_odd > 0 && can_cons_odd())
        sem_post(&m_cons_odd); // Wake up a waiting odd consumer
};
```