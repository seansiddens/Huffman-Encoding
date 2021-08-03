#include "huffman.h"

#include "pq.h"
#include "stack.h"

#include <stdlib.h>

// Constructs a Huffman tree given a computed histogram. Returns the root of the tree.
Node *build_tree(uint64_t hist[static ALPHABET]) {
    PriorityQueue *pq = pq_create(ALPHABET);
    // Enqueue all leaf nodes using histogram
    for (int i = 0; i < ALPHABET; i++) {
        // We only care about symbols that actually appear in our file
        if (hist[i]) {
            enqueue(pq, node_create((uint8_t) i, hist[i]));
        }
    }

    // Dequeue two nodes at a time, join them, then enqueue the joined node back.
    // Repeat until only one node is left in queue. That node is the root
    // of the tree.
    while (pq_size(pq) > 1) {
        // Dequeue two nodes
        Node *left;
        Node *right;
        dequeue(pq, &left);
        dequeue(pq, &right);

        enqueue(pq, node_join(left, right)); // Join them and re-enqueue
    }

    // Get root node from queue
    Node *root;
    dequeue(pq, &root);
    pq_delete(&pq);

    return root;
}

// Populates the code table. Constructed codes are copied to table.
void build_codes(Node *root, Code table[static ALPHABET]) {
    static Code curr_code; // Static structs are zero initialized by default
    // If both children are NULL, then we are at a leaf node
    if (root->left == NULL && root->right == NULL) {
        table[root->symbol] = curr_code; // Add current code to code table
        return;
    } else {
        // Current node is an interior node
        uint8_t popped_bit; // Throwaway variable for storing popped bits

        // Push 0 and recurse left
        code_push_bit(&curr_code, 0);
        build_codes(root->left, table);
        code_pop_bit(&curr_code, &popped_bit); // Pop from code after returning

        // Push 1 and recurse right
        code_push_bit(&curr_code, 1);
        build_codes(root->right, table);
        code_pop_bit(&curr_code, &popped_bit);

        return;
    }
}

// Returns the root node to the tree constructed from the treedump array.
// Iterates over contents of tree dump array and reconstructs the tree
// using a stack of nodes.
Node *rebuild_tree(uint16_t nbytes, uint8_t tree[static nbytes]) {
    Stack *s = stack_create(nbytes);
    // Iterate over tree dump
    for (int i = 0; i < nbytes; i++) {
        // If 'L', next byte is symbol for leaf node
        if (tree[i] == 'L') {
            // Create leaf node, and push onto stack (freq doesn't matter)
            stack_push(s, node_create(tree[i + 1], 1));
            i += 1;
        } else if (tree[i] == 'I') {
            // Pop from stack twice to get right and left child of interior node
            Node *right;
            stack_pop(s, &right);
            Node *left;
            stack_pop(s, &left);

            // Join children nodes and push onto stack
            stack_push(s, node_join(left, right));
        }
    }

    // After iterating, single node left on stack should be the root node
    Node *root;
    stack_pop(s, &root);
    stack_delete(&s); // Delete stack before returning
    return root;
}

// Destructor for Huffman tree
void delete_tree(Node **root) {
    if (*root) {
        if ((*root)->left == NULL && (*root)->right == NULL) {
            // Both children are NULL, so we are at a leaf node
            node_delete(&(*root));
            return;
        } else {
            // Current node is an interior node
            delete_tree(&((*root)->left));
            delete_tree(&((*root)->right));
            node_delete(&(*root));
            return;
        }
    }
}
