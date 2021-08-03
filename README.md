# Assignment 6

---

`encode` compresses a file using the Huffman coding algorithm. `decode` 
will decode compressed files created by `encode`. 

---

## Build Targets

* `make all`

Builds `encode`, `decode`, `error`, and `entropy`.

* `make encode`

Builds the `encode` program

* `make decode`

Builds the `decode` program

* `make entropy`

Builds the `entropy` program

* `make clean`

Removes object and binary files

* `make format`

Formats the source files using clang-format

* `make scan-build`

Analyzes the program for bugs

---

## Running

`./encode [-h] [-v] [-i infile] [-o outfile]`

`./decode [-h] [-v] [-i infile] [-o outfile]`

---

## Command Line arguments:

For the encode program:

- `-h`: Program usage and help.
- `-v`: Print compression statistics
- `-i infile`: Input file to compress (default: stdin).
- `-o outfile`: Output of compressed data (default: stdout).

For the decode program:

- `-h`: Program usage and help.
- `-v`: Print compression statistics
- `-i infile`: Input file to decompress (default: stdin).
- `-o outfile`: Output of decompressed data(default: stdout).

## Bugs

Running scan-build warns of a potential memory leak from `infile_name` and `outfile_name`,
but these strings are freed and their pointers are set to `NULL` when they are no longer needed
and every time the program prematurely exits on error.

It also warns that the value stored to 'bytes' is never read, but this variable
assignment is necessary to for the read_bytes function to update the external variable
used to keep track of the total file size. 

It also warns of potential derencing of null points while traversing the Huffman tree,
but it is traversed in such a way that this cannnot happen.


