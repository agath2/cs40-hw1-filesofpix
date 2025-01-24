/*     
 *     readaline.c
 *     By Agatha Yang, Alina Xie
 *     Date: 1/21/25
 *     
 *     CS40 hw1 FilesofPix
 *
 *     Summary: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "readaline.h"

size_t readaline(FILE *inputfd, char **datapp) {
        if (inpufd == NULL || *datapp == NULL) 
        {
                fprintf(stderr, "Error: Invalid input.");
                exit(EXIT_FAILURE);
        }

        char currChar = fgetc(inputfd);

        if (currChar == EOF) 
        {
                *datapp = NULL;
                return 0;
        }

        long int numBytes = 1;

        // loops through file to get number of bytes in a line
        while (currChar != '\n' && currChar != EOF) 
        {
                currChar = fgetc(inputfd);

                if(currChar != EOF) {
                        numBytes++;
                }
        }

        // allocates memory for the line 
        *datapp = (char*) malloc (numBytes);
        //check if memory allocation succeeded
        if (*datapp == NULL) 
        {
                fprintf(stderr, "Error: Memory allocation failed.")
                exit(EXIT_FAILURE);
        }

        // moves file pointer to start of the line
        fseek(inputfd, -numBytes, SEEK_CUR);

        // reads line 1 char at a time and stores it in datapp
        for(int i = 0; i < numBytes; i++) {
            (*datapp)[i] = (char)fget(inputfd);
        }

        return numBytes;


}