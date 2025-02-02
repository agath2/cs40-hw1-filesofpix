/*     
 *     restoration.c
 *     By Agatha Yang, Alina Xie
 *     Date: 1/21/25
 *     
 *     CS40 hw1 FilesofPix
 *
 *     Summary: This is the .c file of the restoration program. This program 
 *              takes in a corrupted pgm file and restores it to its former 
 *              glory. 
 * 
 *              The main data structures used in this implementation are the 
 *              Hanson table and sequence. The Hason table is used to hold 
 *              key-value pairs of char*s to injections and char*s to the line.
 *              Tables serve the use of finding the recurring injection sequence
 *              indicating an original line of the pgm. 
 *              
 *              The Hanson sequence is used to hold all original lines of the 
 *              pgm. The length of this sequence is equal to the height of the 
 *              pgm. It is a sequence of char*s. 
 * 
 *              The output of this program will be printed to stdout. Inputs
 *              are given as plain pgms (P2), restoration will restore the pgm
 *              to its raw form (P5).
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atom.h"
#include "readaline.h"
#include "table.h"
#include "seq.h"

#include "pnmrdr.h"

#define DEBUG true

int getLength(char **currline, size_t numBytes, int *alphaSize, int *pixSize);
void getPattern(char **currLine, size_t numBytes, int *alphaSize, 
        char **pattern);
void addToSeq(char** ogRow, struct Seq_T* ogRows);
bool isOriginal(char** pattern, const char** ogPattern, int pSize);
void printPgm(struct Seq_T* ogRows, int width, int height);

/* main
 * 
 * Main function for the restoration program
 * 
 * Parameters: 
 *      int argc:     the number of arguments passed to the program 
 *      char *argv[]: the array of all arguments given
 * 
 * Return: return 0 if the program executed successfully; exit(1) if the program
 *      had errors
 * 
 * Expects: the name of the pnm file to be opened 
 * 
 * Note: 
 *      firstOgLine is first set to empty string; when we use Table_put(), if
 *      there are no collisions, Table_put will return NULL and firstOgLine will
 *      be set to NULL. When a collision happens, Table_put() will return the 
 *      line associated with the colliding key. 
 * 
 *      Atoms are used to compare char** addresses and find duplicate char** */
int main(int argc, char *argv[])
{
        FILE *inputFile;

        /* error check */
        if (argc == 1) {
                inputFile = stdin;
        } else {
                inputFile = fopen(argv[1], "r");
        }

        char* firstOgLine = ""; /* first valid line */
        char* currline = NULL;
        char* currPattern = ""; /* current injected chars */
        const char* ogPattern = NULL; /* pattern representing valid line */
        size_t numBytes = 0; /* length of entire row */

        int alphaSize = 0; /* size of injected values */
        int pixSize = 0; /* size of digit values*/

        int height = 0; /* height of the pgm */

        /* table for finding the injection pattern indicating an ogline */
        struct Table_T* uniqueInjs = Table_new(10, NULL, NULL);
        /* sequence containing corrupted og rows */
        struct Seq_T* ogRows = Seq_new(70);

        /* retrieve data from file */
        while (true) 
        {       
                /* read in a new line from the file */
                numBytes = readaline(inputFile, &currline);

                if (numBytes == 0) {
                        /* reading past end of line */
                        if(currline != NULL) {
                                free(currline);
                        }
                        currline = NULL;
                        break;
                }
                
                /* extract injected non-numeric chars */
                if (getLength(&currline, numBytes, &alphaSize, &pixSize) < 0) {
                        /* current line is a fake row and is skipped */
                        if(currline != NULL) {
                                free(currline);
                        }
                        currline = NULL;
                        continue;
                }
                getPattern(&currline, numBytes, &alphaSize, &currPattern); 
                
                if (ogPattern == NULL) {
                        /* we haven't found the injected pattern yet */
                        const char* injected = Atom_new(currPattern, alphaSize);
                        firstOgLine = Table_put(uniqueInjs, injected, currline); 

                        if (firstOgLine != NULL) {
                                /* we have found the ogPattern! */
                                /* record the pattern */
                                ogPattern = injected; 

                                /* add firstOgLine to sequence */
                                addToSeq(&firstOgLine, ogRows);
                                /* add current line to sequence */
                                addToSeq(&currline, ogRows); 

                                /* update height */
                                height = 2; 
                        }
                } else {
                        // we have already found the pattern, horray!
                        if (isOriginal(&currPattern, &ogPattern, alphaSize)) {
                                /* currline is an origial line */
                                height++; 
                                addToSeq(&currline, ogRows);
                        } else {
                                /* currline is not original, release memory */ 
                                Table_remove(uniqueInjs, ogPattern);
                                const char* injected 
                                        = Atom_new(currPattern, alphaSize);
                                Table_put(uniqueInjs, injected, currline);
                                ogPattern = NULL;
                        }
                        
                }
                if (currline != NULL) {
                                free(currline);
                }
                currline = NULL;
                free(currPattern); /* release memory for currPattern */
        }

        printPgm(ogRows, pixSize, height); /* print out pgm */

        Table_free(&uniqueInjs); /* free the table */
        Seq_free(&ogRows); /* free the sequence */
        fclose(inputFile); /* close the file */
}

/* getLength
 * 
 * This function takes in a line and counts the size of both the pixel values 
 *      part of the line AND the injected chars part. It also checks the if
 *      that the digits are valid and not <255.
 * 
 * Parameters:
 *      char** currLine:  pointer to cstring holding current line
 *      size_t numBytes:  the length of the current line
 *      int* alphaSize:   parameter to store result in, implicitly returns the
 *                            size of the injected sequence size
 *      int* pixSize:     parameter to store the length of the pixel data, 
 *                        implicitly returns the size
 * 
 * Return: integer 0 or -1 corresponding to if the row is fake or not. 
 * 
 * Expects: a char** that is not NULL 
 * 
 * Note: This function is structured around making sure it doesn't count every
 *      individual digits of a integer as a separate int needing 4 bytes. */
int getLength(char **currline, size_t numBytes, int *alphaSize, int *pixSize)
{     
        char prevChar = 'b';
        int currPixLen = 0;
        *alphaSize = 0;
        *pixSize = 0;
        /* loop thru current line, count number of injected chars */
        for (size_t i = 0; i < numBytes; i++) 
        {
                if (!isdigit((*currline)[i]) && isdigit(prevChar)) {
                        /* check value of valid digits (0-255) */
                        if (currPixLen >= 3) {
                                int sum = 0;
                                if(currPixLen == 3) {
                                        /* caluculates value of 3 digit number */
                                        sum += (int)(((*currline)[i - 1]) - '0');
                                        sum += (int)((((*currline)[i - 2]) - 
                                               '0') * 10); 
                                        sum += (int)((((*currline)[i - 3]) - '0') * 100);
                                }
                                if(currPixLen >= 4 || sum > 255) {
                                        return -1;
                                }
                        }
                        (*pixSize)++;
                        currPixLen = 0; /* reset the current pixel length */
                }
                /* increment curr length of the nums and chars */
                if (isdigit((*currline)[i])) {
                        currPixLen++;
                } else {
                        (*alphaSize)++;
                }
                prevChar = (*currline)[i]; /* set prev to curr */
        }
        
        if(isdigit((*currline)[numBytes - 1])) {
                (*pixSize)++;
        }

        return 0;
}

/* getPattern
 * 
 * This function takes in a new line of the file and extracts its injections
 *      from the lines 
 * 
 * Parameters: 
 *      char** currline:  A pointer to the cstring of current line read
 *      size_t numBytes:  The length of the line
 *      int* alphaSize:   Pointer to int holding the length of injected chars
 *      char** pattern:   A pointer to the cstring holding injected chars in the 
 *                            line. A parameter to store result in & implicitly
 *                            return.  
 * 
 * Return: Nothing
 * 
 * Expects: Pointers and Pointer-pointers to not be NULL */
void getPattern(char **currLine, size_t numBytes, int *alphaSize, 
        char **pattern)
{
        /* create a cstring for injection patterns */
        *pattern = malloc(*alphaSize + 1);

        size_t patternIndex = 0;

        /* loop through entire line */
        for (size_t i = 0; i < numBytes; i++) {
                if (!isdigit((*currLine)[i])) {
                        /* the current char is an injection, add to string */
                        (*pattern)[patternIndex] = (*currLine)[i];
                        patternIndex++;
                }
        }
        /* add null terminator to the end of the line */
        (*pattern)[patternIndex] = '\0';
}

/* addToSeq
 * 
 * This function receives a corrupted valid line and adds it to a sequence
 * 
 * Parameters:
 *      char** ogLine: a char pointer-pointer to the cstring holding a valid
 *                      line of the pgm
 *      Seq_T* ogRows: a passed-by-reference sequence with char** elements; each
 *                      index holds an ogRow
 * 
 * Return: Nothing.
 * 
 * Expects: char** to not be NULL */
void addToSeq(char** ogRow, struct Seq_T* ogRows)
{    
        /* make a deep copy of the current row */
        size_t length = strlen(*ogRow) + 1; /* add one for null */
        char* lineDup = malloc(length);

        strncpy(lineDup, *ogRow, length); /* copy contents into dup */
        lineDup[length - 1] = '\0';

        Seq_addhi(ogRows, lineDup);
}

/* isOriginal
 * 
 * This function receives a line from the corrupted pgm and checks if it is a
 *      valid pgm row
 * 
 * Parameters:
 *      char** pattern:   Pattern of the current line being inspected
 *      char** ogPattern: Recurring pattern indicating a original pgm row
 * 
 * Returns: A boolean value; true if is ogLine, false otherwise
 * 
 * Expects: both char** to be not NULL */
bool isOriginal(char** pattern, const char** ogPattern, int pSize)
{       
        /* check if they are the same length */
        if (Atom_length(*ogPattern) != pSize) {
                return false;
        }

        for (int i = 0; i < pSize; i++) {
                if ((*pattern)[i] != (*ogPattern)[i]) {
                        /* injected patterns not matching */
                        return false;
                }
        }

        return true;
}

/* printPgm
 * 
 * This function receives sequence of original lines of the pgm and prints out
 *      the resulting pgm
 * 
 * Parameters: 
 *      Seq_T* ogRows: a passed-by-reference sequence with char** elements; each
 *                      index holds an ogRow
 *      int width:     an integer representing width of the pgm
 *      int height:    an integer representing height of the pgm
 * 
 * Return: Nothing. Prints to screen 
 * 
 * Expects: Seq_T pointer to not be NULL
 * 
 * Note: This function also error checks & is responsible for making sure that 
 *      the result is a valid pgm */
void printPgm(struct Seq_T* ogRows, int width, int height)
{
        printf("P5\n%d %d\n255\n", width, height); /* print header */

        char* currline = NULL; /* print raster */
        char prevChar = '\0';
        int pixVal = 0;
        /* loop thru ogRows, print out */
        for (int i = 0; i < Seq_length(ogRows); i++) 
        {       
                /* retreive a line */
                currline = Seq_get(ogRows, i);

                /* loop thru the corrupted line */
                for (int j = 0; (currline)[j] != '\0'; j++) 
                {       
                        if (isdigit((currline)[j])) {
                                /* current char is a digit, add to pixVal */
                                pixVal = (pixVal * 10) + ((currline)[j] - '0'); 
                                
                        } else {
                                /* current char is an injection */
                                if (prevChar != '\0' && isdigit(prevChar)) {
                                        /* a pixel value has just ended */
                                        printf("%c", pixVal); /* print ASCII */
                                        pixVal = 0; /* reset pixVal */
                                }
                        }
                        /* set prev char to current char */
                        prevChar = (currline)[j];
                }
                free(currline);
        }
}