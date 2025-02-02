/*     
 *     readaline.c
 *     By Agatha Yang, Alina Xie
 *     Date: 1/21/25
 *     
 *     CS40 hw1 FilesofPix
 *
 *     Summary: This is the implementation file of the readaline function. This
 *              program receives as parameters an opened FILE and a pointer to a
 *              cstring. Reads in one row of the files at a time, returns number
 *              of bytes of the line, implicitly returns the content of the line
 *              by editing the char* pointed to by datapp.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "readaline.h"

/* readaline
 * 
 * This function reads in a line of a file. Finds the number of bytes of the 
 *      line and returns the line implicitly 
 * 
 * Parameters:
 *      FILE* inputfd: a FILE pointer to the file opened
 *      char** datapp: a pointer to an empty cstring 
 * 
 * Returns: Number of bytes in the line read, type size_t
 * 
 * Expects: Both pointers to not be NULL 
 * 
 * Note: This function iterates over the same line twice; once to count the 
 *      number of bytes, once to store the contents of the line. */
size_t readaline(FILE *inputfd, char **datapp) 
{
        /* error check user input */
        if (inputfd == NULL || datapp == NULL) {
                fprintf(stderr, "Error: Invalid input.\n");
                exit(1);
        }

        char currChar = fgetc(inputfd);
        if (currChar == EOF) { /* edge case: file is at eof */
                *datapp = NULL;
                return 0;
        }

        /* loop thru file to get number of bytes in a line */
        long int numBytes = 1;
        while (currChar != '\n' && currChar != EOF) 
        {
                currChar = fgetc(inputfd);
                if(currChar != EOF) {
                        numBytes++;
                }
        }

        if (numBytes > 1000) {
                fprintf(stderr, "readaline: input line too long\n");
                exit(4);
        }

        *datapp = (char*)malloc (numBytes + 1); /* allocate memory for the line */
        /* check if memory allocation succeeded */
        if (*datapp == NULL) {
                fprintf(stderr, "Error: Memory allocation failed.\n");
                exit(1);
        }

        /* move file pointer back to start of the line */
        fseek(inputfd, -(numBytes + 1), SEEK_CUR);
        /* read line one char at a time and store it in datapp */
        for (int i = 0; i < (numBytes + 1); i++) {
                (*datapp)[i] = (char)fgetc(inputfd);
        }

        (*datapp)[numBytes] = '\0';
        return numBytes;
}