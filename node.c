#include "node.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

//
// Constructor for a node. Returns a pointer to a newly created node
// w/ specified symbol and frequency.
// Pointers to children nodes are initialized to NULL.
//
Node *node_create(uint8_t symbol, uint64_t frequency) {
    Node *n = (Node *) calloc(1, sizeof(Node));
    assert(n != NULL);
    if (n) {
        n->left = NULL;
        n->right = NULL;
        n->symbol = symbol;
        n->frequency = frequency;
    }
    return n;
}

//
// Destructor for a node. Sets children pointers to NULL, frees allocated memory,
// and sets node pointer to NULL.
// Warning: Children nodes are not destroyed
//
void node_delete(Node **n) {
    if (*n) {
        (*n)->left = NULL;
        (*n)->right = NULL;
        free(*n);
        *n = NULL;
    }
    return;
}

//
// Returns a pointer to a parent node who's children are the 'left' and 'right' nodes.
// Parent node's frequency is sum of children, and symbol is '$'.
//
Node *node_join(Node *left, Node *right) {
    Node *n = node_create('$', left->frequency + right->frequency);
    n->left = left;
    n->right = right;
    return n;
}

//
// Prints a node and it's children
//
void node_print(Node *n) {
    // Print parent node
    if (n) {
        printf("(0x%02x, %" PRIu64 ")", n->symbol, n->frequency);
    } else {
        printf("NULL");
    }

    printf(" --> L: ");

    // Print left child
    if (n->left) {
        printf("(0x%x, %" PRIu64 ")", n->left->symbol, n->left->frequency);
    } else {
        printf("NULL");
    }

    printf(" R: ");

    // Print right child
    if (n->right) {
        printf("(0x%x, %" PRIu64 ")", n->right->symbol, n->right->frequency);
    } else {
        printf("NULL");
    }
    printf("\n");

    return;
}
