#include "pq.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// Returns the next position in queue
static inline uint32_t succ(uint32_t pos, uint32_t capacity) {
    return ((pos + 1) % capacity);
}

// Returns the previous position in the queue
static inline uint8_t prev(uint32_t pos, uint32_t capacity) {
    return ((pos + capacity - 1) % capacity);
}

struct PriorityQueue {
    uint32_t head;
    uint32_t tail;
    uint32_t size;
    uint32_t capacity;
    Node **nodes;
};

// Creates a priority queue w/ specified capacity. Returned pointer is null if memory
// allocation fails
PriorityQueue *pq_create(uint32_t capacity) {
    PriorityQueue *pq = (PriorityQueue *) malloc(sizeof(PriorityQueue));
    if (pq) {
        pq->head = 0;
        pq->tail = 0;
        pq->size = 0;
        pq->capacity = capacity + 1; // Add one because full check takes up a space
        pq->nodes = (Node **) calloc(pq->capacity, sizeof(Node *));
        if (!pq->nodes) {
            free(pq);
            pq = NULL;
        }
    }

    return pq;
}

// Deletes queue
void pq_delete(PriorityQueue **q) {
    if (*q && (*q)->nodes) {
        free((*q)->nodes);
        free(*q);
        *q = NULL;
    }
    return;
}

// Returns true if the queue is empty and false otherwise
bool pq_empty(PriorityQueue *q) {
    return q->size ? false : true;
}

// Returns true if the queue is full, and false otherwise
bool pq_full(PriorityQueue *q) {
    // If the next position of the tail is the same as the head, queue is full
    return succ(q->tail, q->capacity) == q->head ? true : false;
}

// Returns the size of the queue
uint32_t pq_size(PriorityQueue *q) {
    return q->size;
}

// Enqueues a node pointer using insertion sort: preserves total ordering of queue
// such that node pointers with the least frequency are dequeued first.
// Returns false if queue is full prior to enqueing, true on success
bool enqueue(PriorityQueue *q, Node *n) {
    // If queue is full we can't enqueue
    if (pq_full(q)) {
        return false;
    }
    if (pq_empty(q)) { // If queue is empty, simply enqueue at open spot
        q->nodes[q->tail] = n;
        q->tail = succ(q->tail, q->capacity); // Get next pos of tail
        q->size++;
    } else {
        uint32_t slot = q->tail;
        // Perform insertion sort of find correct slot to enqueue at
        while (true) {
            if (slot == q->head) {
                q->nodes[slot] = n;
                q->tail = succ(q->tail, q->capacity); // Get next pos of tail
                q->size++;
                break;
            } else if (q->nodes[prev(slot, q->capacity)]->frequency > n->frequency) {
                q->nodes[slot] = q->nodes[prev(slot, q->capacity)];
                slot = prev(slot, q->capacity);
            } else {
                q->nodes[slot] = n;
                q->tail = succ(q->tail, q->capacity); // Get next pos of tail
                q->size++;
                break;
            }
        }
    }
    return true;
}

// Dequeues a node pointer from queue. Frequency of the node should be the
// smallest of the queue.
// Returns false if queue is empty prior to dequeueing, true on success
bool dequeue(PriorityQueue *q, Node **n) {
    // If queue is empty we can't dequeue anything
    if (pq_empty(q)) {
        return false;
    }
    // Otherwise, dequeue from head
    *n = q->nodes[q->head];
    q->head = succ(q->head, q->capacity); // Get next pos for the head
    q->size--; // Keep track of size
    return true;
}

// Print the contents of the queue in dequeue order
void pq_print(PriorityQueue *q) {
    uint32_t i = q->head; // Start at head
    // Continue until we are at tail (tail is empty)
    while (i != q->tail) {
        node_print(q->nodes[i]);
        i = succ(i, q->capacity);
    }
}
