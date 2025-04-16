#ifndef RINGBUFFER_H
#define RINGBUFFER_H

typedef struct {
    short *buffer;
    int head;
    int tail;
    int max;
    bool full;
} RingBuffer;

RingBuffer *ring_buffer_init(int capacity);
void ring_buffer_free(RingBuffer *ring);
bool ring_buffer_is_full(RingBuffer *ring);
bool ring_buffer_is_empty(RingBuffer *ring);
void ring_buffer_put(RingBuffer *ring, short value);
bool ring_buffer_get(RingBuffer *ring, short *value);
int ring_buffer_capacity(RingBuffer *ring);
int ring_buffer_size(RingBuffer *ring);

#endif // RINGBUFFER_H
