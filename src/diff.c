/*
 * diff.c - Simple byte-wise output comparison tool for autograder
 *
 * This program compares the output from stdin (typically user program output)
 * to the contents of a reference output file given as a command-line argument.
 * It reads both sources in fixed-size blocks, counts the number of matching bytes,
 * and returns a percentage score (0-100) indicating how similar the outputs are.
 * If both outputs are empty, it returns 100.
 * Usage: ./diff <reference_output_file>
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

// Macro to get the maximum of two values
#define max(a, b) ((a) > (b) ? (a) : (b))

// Number of bytes to read at a time
#define BYTES_TO_READ 64

int main(int argc , char *argv[]){
    // Check for correct usage
    if (argc != 2) {
        perror("Usage: ./diff <filename>.out\n");
        return 1;
    }
    // Buffers to store bytes read from user output and correct output
    char userbytes[BYTES_TO_READ];
    char correctbytes[BYTES_TO_READ];
    // Open the correct output file for reading
    int correctfile = open(argv[1], O_RDONLY);
    if (correctfile < 0 ) {
        perror("Error opening correct output file");
        return 255;
    }
    int total_userbytesread ,total_correctbytesread;
    int userbytesread = 0,correctbytesread = 0;
    int samebytes = 0 ; // Counter for matching bytes
    int sizeofUbytes = 0, sizeofCbytes = 0; // Total bytes read from user and correct output
    while (1) {
        total_correctbytesread = 0;
        total_userbytesread = 0;
        // Read BYTES_TO_READ bytes from stdin (user output)
        while (total_userbytesread < BYTES_TO_READ) {//make sure to read only BYTES_TO_READ bytes
            userbytesread = read(STDIN_FILENO,userbytes + total_userbytesread, BYTES_TO_READ - total_userbytesread);
            if (userbytesread == 0) { // EOF reached
                break; 
            }
            if (userbytesread < 0) { // Error reading
                close(correctfile);
                return 255;
            }
            total_userbytesread += userbytesread;
        }
        // Read BYTES_TO_READ bytes from correct output file
        while (total_correctbytesread < BYTES_TO_READ) {
            correctbytesread = read(correctfile, correctbytes +total_correctbytesread, BYTES_TO_READ - total_correctbytesread);
            if (correctbytesread == 0) { // EOF reached
                break; 
            }
            if (correctbytesread < 0) { // Error reading
                close(correctfile);
                return 255;
            }
            total_correctbytesread += correctbytesread;
        }
        
        sizeofUbytes += total_userbytesread;
        sizeofCbytes += total_correctbytesread;
        // If no bytes were read from either file, break the loop
        if (total_userbytesread == 0 && total_correctbytesread == 0) {
            break; 
        }   

        // Compare bytes read from both sources
        if (total_userbytesread > 0 && total_correctbytesread > 0) {
            for (int i = 0; (i < total_userbytesread) && (i < total_correctbytesread); i++) {
                if (userbytes[i] == correctbytes[i]) {
                    samebytes++;
                }
            }
        }
    }
    if (close(correctfile) < 0) {
        perror("Error closing correct output file");
        return 255;
    
    }

    // If both files are empty, return 100 (perfect match)
    if (sizeofUbytes == 0 && sizeofCbytes == 0) {
        return 100;
    }
    
    // Calculate the percentage of matching bytes
    unsigned int percentage = (samebytes * 100) / (max(sizeofUbytes, sizeofCbytes));
    return (percentage);
}