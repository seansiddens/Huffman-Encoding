#include "io.h"

#include "defines.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

static uint8_t buffer[BLOCK] = { 0 };
static uint32_t index = 0; // Index of bit in the buffer

// Sets the ith bit of the buffer
void buffer_set_bit(uint32_t i) {
    buffer[i / 8] |= 0x1 << (i % 8);
}

// Clears the ith bit of the buffer
void buffer_clr_bit(uint32_t i) {
    buffer[i / 8] &= ~(0x1 << (i % 8));
}

// Gets the ith bit of the buffer
uint8_t buffer_get_bit(uint32_t i) {
    return 1 & (buffer[i / 8] >> (i % 8));
}

//
// Reads in nbytes from infile, and stores them in buf.
// Looped calls to read() gurantees we read at most nbytes (unless we read EOF).
// Returns the number of bytes read.
//
int read_bytes(int infile, uint8_t *buf, int nbytes) {
    int current_bytes_read = 0;
    int bytes = 0;

    // Loop calls to read() until we've read in nbytes
    while (true) {
        // Read in bytes
        if ((bytes = read(infile, buf + current_bytes_read, nbytes)) <= 0) {
            break;
        }

        current_bytes_read += bytes; // Bytes read in this call
        bytes_read += bytes; // Bytes read total

        if (current_bytes_read == nbytes) {
            break;
        }
    }

    return current_bytes_read;
}

//
// Writes out nybtes from buf
//
int write_bytes(int outfile, uint8_t *buf, int nbytes) {
    int current_bytes_written = 0;
    int bytes = 0;

    // Loop calls to write() until we've written all bytes in buf to outfile
    while (true) {
        // Write bytes out from buffer
        if ((bytes = write(outfile, buf + current_bytes_written, nbytes)) <= 0) {
            break;
        }

        current_bytes_written += bytes; // Bytes written in this call
        bytes_written += bytes; // Bytes written total

        if (current_bytes_written == nbytes) {
            break;
        }
    }

    return current_bytes_written;
}

// Fill bits of infile into  buffer, then return each bit of that buffer. When
// buffer is empty, refill it. Once no more bits can be read from infile, return
// false. Return true if more bits can be read in.
bool read_bit(int infile, uint8_t *bit) {
    if (index == 0) {
        if (read_bytes(infile, buffer, BLOCK) == 0) {
            return false;
        }
    }

    *bit = buffer_get_bit(index);
    index += 1;

    if (index == (BLOCK * 8)) {
        index = 0;
    }

    return true;
}

// Accumulates bits of code in a buffer. When buffer is full, writes
// code bits to outfile.
void write_code(int outfile, Code *c) {
    // Loop over each bit of the code and write to buffer
    uint32_t i = 0;
    while (i < code_size(c)) {
        uint8_t bit = 1 & (c->bits[i / 8] >> (i % 8)); // Get bit of code

        // Either set or clear bit in buffer
        if (bit) {
            buffer_set_bit(index);
        } else {
            buffer_clr_bit(index);
        }

        // Increment counters
        i += 1;
        index += 1;

        // If buffer is full, write buffer out, and reset index
        if (index >= BLOCK * 8) {
            write_bytes(outfile, buffer, BLOCK);
            index = 0;
        }
    }

    return;
}

// Writes remaining code bits left in buffer out to the outfile
void flush_codes(int outfile) {
    // If index is divisible by 8, we write index / 8 bytes of buffer out.
    // If not, then we write (index / 8) + 1 bytes out.
    uint32_t byte_num = (index - 1) % 8 == 0 ? (index - 1) / 8 : ((index - 1) / 8) + 1;
    write_bytes(outfile, buffer, byte_num);
    index = 0; // Reset index to beginning
    return;
}
