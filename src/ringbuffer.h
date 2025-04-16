#ifndef RINGBUFFER_H
#define RINGBUFFER_H

typedef struct RingBuffer RingBuffer;

RingBuffer *ring_buffer_init(int capacity);
void ring_buffer_free(RingBuffer *ring);
bool ring_buffer_is_full(const RingBuffer *ring);
bool ring_buffer_is_empty(const RingBuffer *ring);
void ring_buffer_put(RingBuffer *ring, short value);
bool ring_buffer_get(RingBuffer *ring, short *value);
int ring_buffer_capacity(const RingBuffer *ring);
int ring_buffer_size(const RingBuffer *ring);

#endif // RINGBUFFER_H
