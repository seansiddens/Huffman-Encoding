#include "stack.h"

#include <inttypes.h>
#include <stdlib.h>

//
// Definition of struct Stack, an ADT for a stack data structure which stores
// nodes.
//
// top: Index of next empty slot of the stack
// capacity: Maximum number of items which can be pushed to the stack
// nodes: Array of nodes representing the stack
//
struct Stack {
    uint32_t top; // Index of the next empty slot
    uint32_t capacity; // Number of items that can be pushed to stack
    Node **nodes; // Array of node pointers
};

//
// Constructor function for the stack - returns pointer to newly created stack
//
// capacity: maximum number of nodes the stack can hold
//
Stack *stack_create(uint32_t capacity) {
    Stack *s = (Stack *) malloc(sizeof(Stack));
    if (s) {
        s->top = 0;
        s->capacity = capacity;
        s->nodes = (Node **) calloc(s->capacity, sizeof(Node *));
        if (!s->nodes) {
            free(s);
            s = NULL;
        }
    }
    return s;
}

//
// Destructor function for the stack
//
// s: Pointer to pointer to stack we wish to destroy
//
void stack_delete(Stack **s) {
    if (*s && (*s)->nodes) {
        free((*s)->nodes);
        free(*s);
        *s = NULL;
    }
    return;
}
// -------------------------------------------------------

//
// Returns true if the stack is empty and false otherwise
// If top of stack is at 0, stack is empty
//
// s: Pointer to stack
//
bool stack_empty(Stack *s) {
    return s->top ? false : true;
}

//
// Returns true if stack is full and false otherwise
// If top of stack is equal to capacity, then stack is full
// s: Pointer to stack
//
bool stack_full(Stack *s) {
    return s->top == s->capacity ? true : false;
}

//
// Returns the number of items in the stack
//
// s: Pointer to stack
//
uint32_t stack_size(Stack *s) {
    return s->top;
}

//
// Pushes a node pointer n onto the stack.
// Returns true on success and false on failure (fails if stack is full)
//
// s: Pointer to stack
// n: Pointer to node to push
//
bool stack_push(Stack *s, Node *n) {
    // If stack is full, can't push anything
    if (stack_full(s)) {
        return false;
    }
    // Otherwise, push to stack and increment top
    s->nodes[s->top] = n;
    s->top++;
    return true;
}

//
// Pops an item off the stack, stores it in the pointer n
// Returns true on success and false on failure (when stack is empty when you try to pop)
//
// s: Pointer to stack
// n: Pointer for storing popped node pointer
//
bool stack_pop(Stack *s, Node **n) {
    // If stack is empty, can't pop anything off
    if (stack_empty(s)) {
        return false;
    }
    // Otherwise, return value stored at top - 1 and decrement top
    *n = s->nodes[s->top - 1];
    s->top--;
    return true;
}

//
// Prints the entirety of the stack in pop order
//
// s: Pointer to stack
//
void stack_print(Stack *s) {
    int32_t i = s->top - 1; // Start at top of stack
    while (i >= 0) {
        node_print(s->nodes[i]);
        i -= 1;
    }
    return;
}
