/*     
 *     restoration.c
 *     By Agatha Yang, Alina Xie
 *     Date: 1/21/25
 *     
 *     CS40 hw1 FilesofPix
 *
 *     Summary: 
 * 
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

#define DEBUG true

void getLength(char **currLine, size_t numBytes, int *alphaSize);
void getPattern(char **currLine, size_t numBytes, int *alphaSize, 
        char **pattern);
void addToSeq(char** ogRow, struct Seq_T* ogRows, int height);
bool isOriginal(char** pattern, const char** ogPattern, int pSize);
void printPgm(struct Seq_T* ogRows, int width, int height);

/*** NOTE: should make a struct for storing pgm header info (height, width, etc.) ***/

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
        char* currline = "";
        char* currPattern = ""; /* current injected chars */
        const char* ogPattern = NULL; /* pattern representing valid line */
        size_t numBytes = 0; /* length of entire row */

        int alphaSize = 0; /* size of injected values */

        int height = 0;
        int width = 0; /* width and height of the pgm */

        /* table for finding the injection pattern indicating an ogline */
        struct Table_T* uniqueInjs = Table_new(10, NULL, NULL);
        /* sequence containing corrupted og rows */
        struct Seq_T* ogRows = Seq_new(70);

        /* retrieve data from file */
        while (true) 
        {       
                printf("Gonna read a line!\n");
                /* read in a new line from the file */
                numBytes = readaline(inputFile, &currline);

                if (numBytes == 0) {
                        // reading past end of line
                        break;
                }
                
                /* extract injected non-numeric chars */
                getLength(&currline, numBytes, &alphaSize); 
                getPattern(&currline, numBytes, &alphaSize, &currPattern); 
                
                if (ogPattern == NULL) {
                        /* we haven't found the injected pattern yet */
                        const char* injected = Atom_new(currPattern, alphaSize); /** NOTE: Adds a COPY of the sequence into atom table */
                        firstOgLine = Table_put(uniqueInjs, injected, currline); 

                        if (firstOgLine == NULL) {
                                /* we haven't encountered a collision in table */
                                printf("A unique pattern! %s\n", injected);

                        } else if (*firstOgLine == '\0' && DEBUG) {
                                /* DEBUG CONDITION: if entered this condition, 
                                something is wrong -- ogPattern can only be NULL
                                or a pointer to a valid cstring */
                                printf("ERROR: FirstOgLine is not being updated by Table!\n");
                                exit(1);

                        } else {
                                /* we have found the ogPattern! */
                                printf("Aha! A recurring pattern: %s\n", injected);
                                /* record the pattern */
                                ogPattern = injected; /*** NOTE: REMEMBER ogPattern is an ATOM  ***/
                                width = (int)numBytes - alphaSize; 

                                printf("Calculated width of pgm is %d\n", width);

                                if (ogRows == NULL) {
                                        printf("Sequence pointer is NULL!\n");
                                }

                                printf("About to add first line into seq...\n");
                                /* add firstOgLine to sequence */
                                addToSeq(&firstOgLine, ogRows, 1);
                                /* add current line to sequence */
                                printf("About to add second line into seq...\n");
                                addToSeq(&currline, ogRows, 2); 

                                /* update height */
                                height = 2; 
                                printf("Finished adding first and second ogLines to Seq!\n");
                        }
                } else {
                        // we have already found the pattern, horray!
                        if (isOriginal(&currPattern, &ogPattern, alphaSize)) {
                                /* currline is an origial line */
                                height++; 
                                printf("Found an original line! Going to call addToSeq...\n");
                                addToSeq(&currline, ogRows, height);

                                printf("Returned from addToSeq! Injected Seq: %s. Height is now %d", currPattern, height);
                        } else {
                                /* currline is not original, release memory */ 
                                free(currline);
                        }
                }
                alphaSize = 0; /* reset alphaSize */
                free(currPattern); /* release memory for currPattern */
        }             
        /* print out pgm */
        printPgm(ogRows, width, height);

        Table_free(&uniqueInjs); /* free the table */
        Seq_free(&ogRows); /* free the sequence */
        fclose(inputFile); /* close the file */
}

/* getLength
 * 
 * This function takes in a line and counts the size of both the pixel values 
 *      part of the line AND the injected chars part
 * 
 * Parameters:
 *      char** currLine:  pointer to cstring holding current line
 *      size_t numBytes:  the length of the current line
 *      int* patterSize:  parameter to store result in, implicitly returns the
 *                            size of the injected sequence size
 * 
 * Return: Nothing 
 * 
 * Expects: a char** that is not NULL 
 * 
 * Note: This function is structured around making sure it doesn't count every
 *      individual digits of a integer as a separate int needing 4 bytes. */
void getLength(char **currline, size_t numBytes, int *alphaSize)
{     
        printf("Entered getLength!\n");      
        /* loop thru current line, count number of injected chars */
        for (size_t i = 0; i < numBytes; i++) 
        {
                if (!isdigit((*currline)[i])) {
                        /* current char is an injection */
                        (*alphaSize)++;
                }
        }
        printf("Going to exit from getLength, pattern size is %d\n", (*alphaSize));
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
 *      int height:    The height of the pgm so far.
 * 
 * Return: Nothing.
 * 
 * Expects: char** to not be NULL */
void addToSeq(char** ogRow, struct Seq_T* ogRows, int height)
{       
        printf("In addToSeq()!\n");

        /* make a deep copy of the current row */
        size_t length = strlen(*ogRow) + 1; /* add one for null */
        char* lineDup = (char*)malloc(length);
        strcpy(lineDup, *ogRow); /* copy contents into dup */

        Seq_addhi(ogRows, &lineDup);
        // Seq_put(ogRows, height, &ogRow); /* height - 1, convert to index */

        if (DEBUG) {
                printf("Added line %s to index %d in Seq!\n", *ogRow, height);
        }
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
        /* print raster */
        char** currline = NULL;
        char prevChar = '\0';
        int pixVal = 0;

        printf("About to print raster, size of seq is %d\n", Seq_length(ogRows));

        /* loop thru ogRows, print out */
        for (int i = 0; i < Seq_length(ogRows); i++) 
        {       
                printf("Read in another row, seq index %d\n", i);
                if (currline == NULL) {
                        printf("Before calling Seq_get, currline is NULL\n");
                }
                
                /* retreive a line */
                currline = Seq_get(ogRows, i);
                
                if (currline == NULL) {
                        printf("CURRLINE IS NULL\n");
                }

                printf("Currline contents: %s\n", (*currline));
                if (currline == NULL) {
                        printf("The line we are about to print doesn't exist haha\n");
                }

                /* loop thru the corrupted line */
                char currchar = (*currline)[0];
                printf("The first char in the line is: %c\n", currchar);
                for (int j = 0; (*currline)[j] != '\0'; j++) 
                {       
                        printf("Entered for loop, index at is %d\n", j);
                        if (isdigit((*currline)[j])) {
                                /* current char is a digit, add to pixVal */
                                pixVal = (pixVal * 10) + (*currline)[j]; 
                        } else {
                                /* current char is an injection */
                                if (prevChar != '\0' && isdigit(prevChar)) {
                                        /* a pixel value has just ended */

                                        if (pixVal > 255) { /* errorcheck */
                                                printf("Invalid pix value!\n");
                                                exit(1);
                                        }
                                
                                        printf("%c", pixVal); /* print ASCII */
                                        pixVal = 0; /* reset pixVal */
                                }
                                /* add a newline if we're at the end of a row */
                                if ((*currline)[j] == '\n') {
                                        printf("\n");
                                }
                        }
                        /* set prev char to current char */
                        prevChar = (*currline)[j];
                }
        }
}


