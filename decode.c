//#define DEBUG

#include "defines.h"
#include "header.h"
#include "huffman.h"
#include "io.h"

#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define OPTIONS "hvi:o:"

void print_help() {
    printf("SYNOPSIS\n");
    printf("  A Huffman decoder.\n");
    printf("  Decompresses a file using the Huffman coding algorithm.\n\n");
    printf("USAGE\n");
    printf("  ./decode [-h] [-i infile] [-o outfile]\n\n");
    printf("OPTIONS\n");
    printf("  -h             Program usage and help.\n");
    printf("  -v             Print compression statistics.\n");
    printf("  -i infile      Input file to decompress.\n");
    printf("  -o outfile     Output of decompressed data.\n");
    return;
}

// Initialize global variables for tracking file sizes
uint64_t bytes_read = 0;
uint64_t bytes_written = 0;

int main(int argc, char *argv[]) {
    // Argument flags
    bool HELP = false;
    bool VERBOSE = false;

    // Initialize default values
    char *infile_name = NULL;
    char *outfile_name = NULL;
    int infile = STDIN_FILENO;
    int outfile = STDIN_FILENO;

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

    // If an input name is supplied, open the file for reading
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

    // Process header from infile
    Header header;
    read_bytes(infile, (uint8_t *) &header, sizeof(Header));

    if (header.magic != MAGIC) {
        fprintf(stderr, "Invalid magic number.\n");
        free(infile_name);
        free(outfile_name);
        exit(1);
    }
#ifdef DEBUG
    printf("Header values\n");
    printf("Magic number: %8x\n", header.magic);
    printf("Permissions:  %4x\n", header.permissions);
    printf("Tree size: %" PRIu16 " bytes\n", header.tree_size);
    printf("File size: %" PRIu64 " bytes\n\n", header.file_size); // Uncompressed file size
#endif

    // Store tree dump in array
    uint8_t *tree_dump = (uint8_t *) calloc(header.tree_size, sizeof(uint8_t));
    read_bytes(infile, tree_dump, header.tree_size);
    // Reconstruct tree from tree dump
    Node *root = rebuild_tree(header.tree_size, tree_dump);

    // Buffer for storing decoded symbols to eventually write out
    uint8_t *out_buf = (uint8_t *) calloc(header.file_size, sizeof(uint8_t));

    // Read in bits of input file and decode by traversing tree
    uint8_t bit;
    Node *curr_node = root; // Keep track of current node in tree
    uint32_t symbols_written = 0; // Keep track of how many symbols we've written to buffer
    while (true) {
        // Read in bit
        read_bit(infile, &bit);

        // Traverse tree
        if (bit) {
            curr_node = curr_node->right;
        } else {
            curr_node = curr_node->left;
        }

        // Check if we are at leaf
        if ((curr_node->left == NULL) || (curr_node->right == NULL)) {
            out_buf[symbols_written] = curr_node->symbol; // Write symbol to buffer
            symbols_written += 1;
            curr_node = root; // Set node back to root
        }

        // Check if we're done writing
        if (symbols_written == header.file_size) {
            break;
        }
    }
    write_bytes(outfile, out_buf, header.file_size); // Write buffer out to file

    // Print statistics
    if (VERBOSE) {
        fprintf(stderr, "Compressed file size: %" PRIu64 " bytes \n", bytes_read);
        fprintf(stderr, "Decompressed file size: %" PRIu64 " bytes \n", header.file_size);

        float space_saving = 1.0 - (bytes_read / (double) header.file_size);
        fprintf(stderr, "Space saving:  %.2f%% \n", 100.0 * space_saving);
    }

    // Free everything
    delete_tree(&root);
    free(tree_dump);
    free(out_buf);
    free(infile_name);
    free(outfile_name);
    return 0;
}
