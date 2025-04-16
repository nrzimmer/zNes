#include "ringbuffer.h"

#include <stdlib.h>

RingBuffer *ring_buffer_init(int capacity) {
    RingBuffer *ring = malloc(sizeof(RingBuffer));
    if (!ring)
        return nullptr;
    ring->buffer = (short *)malloc(sizeof(short) * capacity);
    if (!ring->buffer) {
        free(ring);
        return nullptr;
    }
    ring->max = capacity;
    ring->head = 0;
    ring->tail = 0;
    ring->full = false;
    return ring;
}

void ring_buffer_free(RingBuffer *ring) {
    free(ring->buffer);
    free(ring);
}

bool ring_buffer_is_full(RingBuffer *ring) { return ring->full; }

bool ring_buffer_is_empty(RingBuffer *ring) { return (!ring->full && (ring->head == ring->tail)); }

void ring_buffer_put(RingBuffer *ring, short value) {
    ring->buffer[ring->head] = value;
    if (ring->full) {
        ring->tail = (ring->tail + 1) % ring->max; // Overwrite oldest data
    }
    ring->head = (ring->head + 1) % ring->max;
    ring->full = (ring->head == ring->tail);
}

bool ring_buffer_get(RingBuffer *ring, short *value) {
    if (ring_buffer_is_empty(ring)) {
        return false;
    }

    *value = ring->buffer[ring->tail];
    ring->tail = (ring->tail + 1) % ring->max;
    ring->full = false;
    return true;
}

int ring_buffer_capacity(RingBuffer *ring) { return ring->max; }

int ring_buffer_size(RingBuffer *ring) {
    if (ring->full) {
        return ring->max;
    }

    if (ring->head >= ring->tail) {
        return ring->head - ring->tail;
    }

    return ring->max + ring->head - ring->tail;
}
