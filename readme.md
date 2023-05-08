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
    if (!cond()) {
        (*wait_cnt)++;     // Increment the counter
        sem_post(&m_fifo); // Release the lock on the fifo
        sem_wait(m);       // Wait for the condition to be met
        (*wait_cnt)--;     // Decrement the counter
        // The thread that sends signal on the "m" mutex
        // is expected to acquire the fifo mutex
        // so at this point the mutex is locked
        // and there is no need to lock it again
    }
};
```
### Update routine
This routine is called after every change to the fifo.  
It checks if any of the waiting threads can be woken up and wakes them up.  
```C
void update_semaphores() {
    // If there is a thread to wake up then wake it up
    // it is now that thread's responsibility to unlock the mutex
    // If there is no thread to wake up then release the mutex
    if (wait_cnt_prod_even > 0 && can_prod_even()) {
        sem_post(&m_prod_even);
    } else if (wait_cnt_prod_odd > 0 && can_prod_odd()) {
        sem_post(&m_prod_odd);
    } else if (wait_cnt_cons_even > 0 && can_cons_even()) {
        sem_post(&m_cons_even);
    } else if (wait_cnt_cons_odd > 0 && can_cons_odd()) {
        sem_post(&m_cons_odd);
    } else {
        sem_post(&m_fifo);
    }
};
```