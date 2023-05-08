#include <fifo.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* ---------------------------- Global variables ---------------------------- */

fifo_t fifo;  // The fifo that will be used by the producers and consumers

/* ----------------------------- Synchronization ---------------------------- */

sem_t m_fifo;       // Mutex that locks access to the fifo
sem_t m_prod_even;  // Mutex that wakes up a waiting even producer
sem_t m_prod_odd;   // Mutex that wakes up a waiting odd producer
sem_t m_cons_even;  // Mutex that wakes up a waiting even consumer
sem_t m_cons_odd;   // Mutex that wakes up a waiting odd consumer

int wait_cnt_prod_even = 0;  // Number of waiting even producers
int wait_cnt_prod_odd = 0;   // Number of waiting odd producers
int wait_cnt_cons_even = 0;  // Number of waiting even consumers
int wait_cnt_cons_odd = 0;   // Number of waiting odd consumers

/* ------------------------------- Conditions ------------------------------- */

// Returns 1 if a producer can produce an even number, 0 otherwise
int can_prod_even() {
    return fifo_count(&fifo) < fifo.size && fifo_count_even(&fifo) < 10;
};

// Returns 1 if a producer can produce an odd number, 0 otherwise
int can_prod_odd() {
    return fifo_count(&fifo) < fifo.size &&
           fifo_count_odd(&fifo) < fifo_count_even(&fifo);
};

// Returns 1 if a consumer can consume an even number, 0 otherwise
int can_cons_even() {
    return fifo_top(&fifo) % 2 == 0 && fifo_count(&fifo) > 3;
};

// Returns 1 if a consumer can consume an odd number, 0 otherwise
int can_cons_odd() {
    return fifo_top(&fifo) % 2 == 1 && fifo_count(&fifo) > 7;
};

/* ------------------------- Synchronization update ------------------------- */

// Updates the semaphores based on the current state of the program
void update_semaphores() {
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

// Waits for a condition to be true, and updates the semaphores
void wait_for_condition(int (*cond)(), sem_t* m, int* wait_cnt) {
    if (!cond()) {
        (*wait_cnt)++;
        sem_post(&m_fifo);
        sem_wait(m);
        (*wait_cnt)--;
    }
};

/* --------------------------------- Actions -------------------------------- */

// Pushes an even number to the fifo
void prod_even(int num) {
    sem_wait(&m_fifo);
    wait_for_condition(can_prod_even, &m_prod_even, &wait_cnt_prod_even);
    fifo_push(&fifo, num);
    update_semaphores();
};

// Pushes an odd number to the fifo
void prod_odd(int num) {
    sem_wait(&m_fifo);
    wait_for_condition(can_prod_odd, &m_prod_odd, &wait_cnt_prod_odd);
    fifo_push(&fifo, num);
    update_semaphores();
};

// Pops an even number from the fifo
int cons_even() {
    sem_wait(&m_fifo);
    wait_for_condition(can_cons_even, &m_cons_even, &wait_cnt_cons_even);
    int num = fifo_pop(&fifo);
    update_semaphores();
    return num;
};

// Pops an odd number from the fifo
int cons_odd() {
    sem_wait(&m_fifo);
    wait_for_condition(can_cons_odd, &m_cons_odd, &wait_cnt_cons_odd);
    int num = fifo_pop(&fifo);
    update_semaphores();
    return num;
};

/* -------------------------------- Threads -------------------------------- */

// Producer thread that produces even numbers
void* a1(void* arg) {
    int num = 0;
    while (1) {
        prod_even(num);
        num += 2;
        num %= 50;
    }
    return NULL;
};

// Producer thread that produces odd numbers
void* a2(void* arg) {
    int num = 1;
    while (1) {
        prod_odd(num);
        num += 2;
        num %= 50;
    }
    return NULL;
};

// Consumer thread that consumes even numbers
void* b1(void* arg) {
    while (1) {
        cons_even();
    }
    return NULL;
};

// Consumer thread that consumes odd numbers
void* b2(void* arg) {
    while (1) {
        cons_odd();
    }
    return NULL;
};

/* ---------------------------------- Setup --------------------------------- */

// Initializes the semaphores and the FIFO
void init(int fifo_size) {
    // Initialize the FIFO
    fifo.buf = malloc(fifo_size * sizeof(int));
    fifo.head = 0;
    fifo.tail = 0;
    fifo.size = fifo_size;

    // Initialize the semaphores
    sem_init(&m_fifo, 0, 1);
    sem_init(&m_prod_even, 0, 0);
    sem_init(&m_prod_odd, 0, 0);
    sem_init(&m_cons_even, 0, 0);
    sem_init(&m_cons_odd, 0, 0);

    // Clear wait counts
    wait_cnt_prod_even = 0;
    wait_cnt_prod_odd = 0;
    wait_cnt_cons_even = 0;
    wait_cnt_cons_odd = 0;
};

// Destroys the semaphores and the FIFO
void cleanup() {
    // Destroy the semaphores
    sem_destroy(&m_fifo);
    sem_destroy(&m_prod_even);
    sem_destroy(&m_prod_odd);
    sem_destroy(&m_cons_even);
    sem_destroy(&m_cons_odd);

    // Destroy the FIFO
    free(fifo.buf);
};

/* ---------------------------------- Tests --------------------------------- */

// TEST1
//  Only a1 is running
//  Should produce 10 even numbers
void test_1() {
    printf("Test 1\n");
    init(11);
    pthread_t thread_a1;
    pthread_create(&thread_a1, NULL, a1, NULL);
    sleep(1);
    if (fifo_count(&fifo) != 10 || fifo_count_even(&fifo) != 10) {
        printf("FAILED\n");
    } else {
        printf("OK\n");
    }
    pthread_cancel(thread_a1);
    cleanup();
};

// TEST2
// Only a2 is running
// Shouldn't produce any numbers
void test_2() {
    printf("Test 2\n");
    init(1);
    pthread_t thread_a2;
    pthread_create(&thread_a2, NULL, a2, NULL);
    sleep(1);
    if (fifo_count(&fifo) != 0) {
        printf("FAILED\n");
    } else {
        printf("OK\n");
    }
    pthread_cancel(thread_a2);
    cleanup();
};

// TEST3
// Only b1 is running
// Shouldn't consume any numbers
void test_3() {
    printf("Test 3\n");
    init(1);
    pthread_t thread_b1;
    pthread_create(&thread_b1, NULL, b1, NULL);
    sleep(1);
    if (fifo_count(&fifo) != 0) {
        printf("FAILED\n");
    } else {
        printf("OK\n");
    }
    pthread_cancel(thread_b1);
    cleanup();
};

// TEST4
// Only b2 is running
// Shouldn't consume any numbers
void test_4() {
    printf("Test 4\n");
    init(1);
    pthread_t thread_b2;
    pthread_create(&thread_b2, NULL, b2, NULL);
    sleep(1);
    if (fifo_count(&fifo) != 0) {
        printf("FAILED\n");
    } else {
        printf("OK\n");
    }
    pthread_cancel(thread_b2);
    cleanup();
};

// TEST5
// a1 and a2 are running
// Should produce 20 numbers
void test_5() {
    printf("Test 5\n");
    init(21);
    pthread_t thread_a1;
    pthread_t thread_a2;
    pthread_create(&thread_a1, NULL, a1, NULL);
    pthread_create(&thread_a2, NULL, a2, NULL);
    sleep(1);
    if (fifo_count(&fifo) != 20) {
        printf("FAILED, %d != 20\n", fifo_count(&fifo));
    } else {
        printf("OK\n");
    }
    pthread_cancel(thread_a1);
    pthread_cancel(thread_a2);
    cleanup();
};

/* ---------------------------------- Main ---------------------------------- */

// Run all the tests
int main(void) {
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
}