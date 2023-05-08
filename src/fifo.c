#include <fifo.h>

void fifo_push(fifo_t* f, int data) {
    f->buf[f->tail] = data;
    f->tail = (f->tail + 1) % f->size;
};

int fifo_pop(fifo_t* f) {
    int data = f->buf[f->head];
    f->head = (f->head + 1) % f->size;
    return data;
};

int fifo_top(fifo_t* f) {
    return f->buf[f->head];
};

int fifo_count(fifo_t* f) {
    return (f->tail - f->head + f->size) % f->size;
};

int fifo_count_even(fifo_t* f) {
    int count = 0;
    for (int i = f->head; i != f->tail; i = (i + 1) % f->size) {
        if (f->buf[i] % 2 == 0) {
            count++;
        }
    }
    return count;
};

int fifo_count_odd(fifo_t* f) {
    return fifo_count(f) - fifo_count_even(f);
};