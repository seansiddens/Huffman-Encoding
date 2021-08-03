//#define DEBUG

#include "code.h"
#include "defines.h"
#include "header.h"
#include "huffman.h"
#include "io.h"
#include "node.h"
#include "pq.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define OPTIONS "hvi:o:"

// Initialize global variables for tracking file sizes
uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

// Prints the program usage and help message
static void print_help(void) {
    printf("SYNOPSIS\n");
    printf("  A Huffman encoder.\n");
    printf("  Compresses a file using the Huffman coding algorithm.\n");
    printf("\n");
    printf("USAGE\n");
    printf("  ./encode [-h] [-i infile] [-o outfile]\n");
    printf("\n");
    printf("OPTIONS\n");
    printf("  -h             Program usage and help.\n");
    printf("  -v             Print compression statistics.\n");
    printf("  -i infile      Input file to compress.\n");
    printf("  -o outfile     Output of compressed data.\n");
    return;
}

// Writes Huffman tree to the tree_dump array
void tree_dump(Node *root, uint8_t *tree_buf) {
    static uint32_t index; // Keep track of where we are in the tree_dump array
    // If both children are NULL, then we are at a leaf node
    if (root->left == NULL && root->right == NULL) {
        // Write an 'L' followed by the symbol
        tree_buf[index] = 'L';
        tree_buf[index + 1] = root->symbol;
        index += 2;
        return;
    } else {
        // Current node is an interior node
        tree_dump(root->left, tree_buf); // Recurse left
        tree_dump(root->right, tree_buf); // Recurse right

        // Write an 'I' to buffer
        tree_buf[index] = 'I';
        index += 1;
        return;
    }
}

// Iterates through code table and prints the corresponding code for each symobl
void print_codes(Code table[static ALPHABET]) {
    printf("\nCode table: \n");
    for (int i = 0; i < ALPHABET; i++) {
        if (!code_empty(&table[i])) {
            printf("0x%02x: ", i);
            code_print(&table[i]);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    // Argument flags
    bool HELP = false;
    bool VERBOSE = false;

    // Initialize default values
    char *infile_name = NULL;
    char *outfile_name = NULL;
    int infile = STDIN_FILENO;
    int outfile = STDOUT_FILENO;

    // Process command line arguments
    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': HELP = true; break;
        case 'v': VERBOSE = true; break;
        case 'i': infile_name = strdup(optarg); break;
        case 'o': outfile_name = strdup(optarg); break;
        default: HELP = true; break;
        }
    }

    // If help option is supplied, print help message and exit program
    if (HELP) {
        print_help();
        free(infile_name);
        free(outfile_name);
        return 0;
    }

    // If an input file name is suplied, open the file for reading
    if (infile_name != NULL) {
        if ((infile = open(infile_name, O_RDONLY)) == -1) {
            fprintf(stderr, "Invalid file name!\n");
            free(infile_name);
            free(outfile_name);
            exit(1);
        }
    }

    // Change permissions of output file to match input
    struct stat statbuf;
    fstat(infile, &statbuf);
    if (outfile_name != NULL) {
        outfile = open(outfile_name, O_WRONLY | O_CREAT | O_TRUNC, statbuf.st_mode);
        fchmod(outfile, statbuf.st_mode);
    }

    // Histogram for storing # of occurences of each byte
    uint64_t histogram[ALPHABET] = { 0 };
    // Increment 0 and 255 so that min of two things are present
    histogram[0] += 1;
    histogram[255] += 1;

    // Buffer for I/O
    uint8_t buffer[BLOCK] = { 0 };

    // Read in all bytes from input
    int bytes;
    while ((bytes = read_bytes(infile, buffer, BLOCK)) != 0) {

        // Go through bytes read in, and increment histogram
        for (int i = 0; i < bytes; i++) {
            histogram[buffer[i]] += 1;
        }
    }
    uint64_t uncompressed_file_size = bytes_read; // Uncompressed file size is

    // Go through histogram and figure out the number of unique symbols
    uint32_t unique_symbols = 0;
    for (int i = 0; i < ALPHABET; i++) {
        if (histogram[i]) { // If histogram is non-zero, increment symbol count
            unique_symbols += 1;
        }
    }
#ifdef DEBUG
    // Print histogram
    printf("Histogram:\n");
    for (int i = 0; i < ALPHABET; i++) {
        if (histogram[i]) {
            printf("0x%02x: %" PRIu64 "\n", i, histogram[i]);
        }
    }
    printf("\n");
#endif

    // Build tree from histogram
    Node *root = build_tree(histogram);

    // Populate code table
    Code code_table[ALPHABET] = { 0 };
    build_codes(root, code_table);
#ifdef DEBUG
    print_codes(code_table);
#endif

    // Create header
    Header header;
    header.magic = MAGIC;
    header.permissions = statbuf.st_mode;
    header.tree_size = (3 * unique_symbols) - 1;
    header.file_size = bytes_read;
#ifdef DEBUG
    // Print header values
    printf("Header values\n");
    printf("Magic number: %8x\n", header.magic);
    printf("Permissions:  %4x\n", header.permissions);
    printf("Tree size: %" PRIu16 " bytes\n", header.tree_size);
    printf("File size: %" PRIu64 " bytes\n\n", header.file_size);
#endif

    write_bytes(outfile, (uint8_t *) &header, sizeof(header));

    // Create buffer to store tree dump
    uint8_t *tree_buf = (uint8_t *) calloc(header.tree_size, sizeof(uint8_t));
    tree_dump(root, tree_buf); // Dump tree to buffer
    // Write tree dump to outfile
    write_bytes(outfile, tree_buf, header.tree_size);

    // Start at beginning of infile and write out all of the codes
    lseek(infile, 0, SEEK_SET);
    bytes = 0;
    while ((bytes = read_bytes(infile, buffer, BLOCK)) != 0) {
        for (int i = 0; i < bytes; i++) {
            Code *c = &code_table[buffer[i]];
            write_code(outfile, c);
        }
    }
    flush_codes(outfile);

    uint64_t compressed_file_size = bytes_written;

    // Print statistics
    if (VERBOSE) {
        fprintf(stderr, "Uncompressed file size: %" PRIu64 " bytes \n", uncompressed_file_size);
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes \n", compressed_file_size);

        float space_saving = 1.0 - (compressed_file_size / (double) uncompressed_file_size);
        fprintf(stderr, "Space saving:  %.2f%% \n", 100.0 * space_saving);
    }

    // Deallocate memory and close file streams
    free(tree_buf);
    delete_tree(&root);
    free(infile_name);
    free(outfile_name);
    close(infile);
    close(outfile);
    return 0;
}
