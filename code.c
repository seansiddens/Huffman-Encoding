#include "code.h"

#include <stdio.h>

// Sets the ith bit of the bit stack
void code_set_bit(Code *c, uint32_t i) {
    c->bits[i / 8] |= 0x1 << (i % 8);
}

// Clears the ith bit of the bit stack
void code_clr_bit(Code *c, uint32_t i) {
    c->bits[i / 8] &= ~(0x1 << (i % 8));
}

// Gets the ith bit of the buffer
uint8_t code_get_bit(Code *c, uint32_t i) {
    return 1 & (c->bits[i / 8] >> (i % 8));
}

// Initializes a new code. Doesn't dynamically allocate any memory.
Code code_init(void) {
    Code c;
    c.top = 0;
    // Zero out bits array
    for (int i = 0; i < MAX_CODE_SIZE; i++) {
        c.bits[i] = 0;
    }

    return c;
}

// Returns size of code, which is equivalent to # of bits pushed onto the Code
uint32_t code_size(Code *c) {
    return c->top;
}

// Returns true if the code is empty, false otherwise
bool code_empty(Code *c) {
    return c->top ? false : true;
}

// Returns true if the code is full, false otherwise.
// Maximum number of bits for a code is 256 (ALPHABET)
bool code_full(Code *c) {
    return c->top == ALPHABET ? true : false;
}

// Pushes a bit to the code stack
bool code_push_bit(Code *c, uint8_t bit) {
    // If code stack is full, can't push anything
    if (code_full(c)) {
        return false;
    } else {
        // Set the bit at c->top to the value of bit.
        if (bit) {
            code_set_bit(c, c->top);
        } else {
            code_clr_bit(c, c->top);
        }
        c->top += 1;
        return true;
    }
}

// Pops bit off of code stack, passing back the value through bit
bool code_pop_bit(Code *c, uint8_t *bit) {
    // If code stack is empty, can't pop anything
    if (code_empty(c)) {
        return false;
    } else {
        // Return the value of the bit at c->top and then decrement c->top
        *bit = code_get_bit(c, c->top - 1);
        c->top -= 1;
        return true;
    }
}

// Print out bits in code order e.g order you'd take to traverse the code tree
void code_print(Code *c) {
    // If code is empty, nothing to print
    if (code_empty(c)) {
        return;
    } else {
        // Start at top of stack, and print each bit going down
        for (int i = 0; i < (int) c->top; i++) {
            printf("%c", '0' + code_get_bit(c, i)); // Gets each bit
        }
    }
    printf("\n");

    return;
}
