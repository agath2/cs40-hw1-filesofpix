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
#include <stdio.h>
#include <stdlib.h>
#include "atom.h"
#include "readaline.h"
#include "table.h"
#include "seq.h"

#define DEBUG true; 

void getLength(char **currLine, size_t numBytes, int *pixValsSize, 
        int *patternSize);
void getPattern(char **currLine, size_t numBytes, int *alphaSize, 
        char **pattern);
void addToSeq(char** ogRow, Seq_T* ogRows, int height);
bool isOriginal(char** pattern, char** ogPattern, int pSize);
void printPgm(Seq_T* ogRows, int width, int height);

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
 * Expects: the name of the pnm file to be opened */
int main(int argc, char *argv[])
{
        FILE *inputFile;
        
        // error check
        if (argc == 1) {
                inputFile = stdin;
        } else {
                inputFile = fopen(argv[1], "r");
        }

        char* firstOgLine = ""; // cstring to first valid line
        char* currline = "";
        char* currPattern = ""; // cstring to current injected chars
        char* ogPattern = NULL; // cstring to pattern representing valid line
        size_t numBytes = 0; // length of entire row

        int alphaSize = 0; // size of injected values

        int height = 0;
        int width = 0; // width and height of the pgm 

        // table for finding the injectino pattern indicating an original line
        Table_T uniqueInjs = Table_new(10, NULL, NULL);
        // sequence containing corrupted og rows
        Seq_T ogRows = Seq_new(70);

        // retrieve data from file 
        while (fgetc(inputFile) != EOF) 
        {       
                // read in a new line from the file
                numBytes = readaline(inputFile, &currline);
                
                // extract injected non-numeric chars
                getLength(&currline, numBytes, &pixSize, &alphaSize); 
                getPattern(&currline, numBytes, &pixSize, &alphaSize, 
                        &currPixVals, &currPattern); 
                
                if (ogPattern == NULL) {
                        // we haven't found the injected pattern yet
                        // create an atom for injected chars
                        char* injected = Atom_new(currPattern, alphaSize); 
                        firstOgLine = Table_put(uniqueInjs, injected, currline); 
                        /*** NOTE: will currline get deleted? How will memory be released? ***/

                        if (firstOgLine == NULL) {
                                // we haven't encountered a collision in table
                                printf("A unique pattern! %s\n", injected);

                        } else if (*firstOgLine == "" && DEBUG) {
                                /* DEBUG CONDITION: if entered this condition, 
                                something is wrong -- ogPattern can only be NULL
                                or a pointer to a valid cstring */
                                printf("ERROR: FirstOgLine is not being updated by Table!\n");
                                exit(1);

                        } else {
                                // we have found the ogPattern!
                                printf("Aha! A recurring pattern: %s\n", injected);
                                // record the pattern
                                ogPattern = injected /*** NOTE: REMEMBER ogPattern is an ATOM  ***/

                                // add firstOgLine to sequence
                                addToSeq(&firstOgLine, &ogRows, 1);
                                // add current line to sequence
                                addToSeq(&currline, &ogRows, 2); 

                                // update height
                                height = 2; 
                                printf("Finished adding first and second ogLines to Seq!\n");
                        }
                } else {
                        // we have already found the pattern, horray!
                        if (isOriginal(&currPattern, &ogPattern, alphaSize)) {
                                // currline is an origial line
                                height++; 
                                addToSeq(&currline, &ogRows, height);

                                printf("Found an original line! Injected Seq: %s. Height is now %d", currPattern, height);
                        }
                }
        }             
        // we have now reached end of line
        // errorcheck and see if the pgm file we have right now is valid
        validOrNot();
        // valid, print out
        printPnm();
        
        if (DEBUG) {
                numBytes = readaline(inputFile, &currline);
                getLength(&currline, numBytes, &pixSize, &alphaSize);
                getPattern(&currline, numBytes, &alphaSize, &currPattern); 
        }

        Seq_free(&uniqueInjs); // free the sequence
        fclose(inputFile); // close the file
}

/* getLength
 * 
 * This function takes in a line and counts the size of both the pixel values 
 *      part of the line AND the injected chars part
 * 
 * Parameters:
 *      char** currLine:  pointer to cstring holding current line
 *      size_t numBytes:  the length of the current line
 *      int* pixValsSize: parameter to store result in, implicitly returns the
 *                            size of the pixel values size
 *      int* patterSize:  parameter to store result in, implicitly returns the
 *                            size of the injected sequence size
 * 
 * Return: Nothing 
 * 
 * Expects: a char** that is not NULL 
 * 
 * Note: This function is structured around making sure it doesn't count every
 *      individual digits of a integer as a separate int needing 4 bytes. */
void getLength(char **currLine, size_t numBytes, int *pixSize, 
                int *alphaSize)
{
        char prev = (*currLine)[0];
        
        // loops through entire line
        for (size_t i = 0; i < numBytes; i++) {
                if (!isdigit((*currLine)[i]) && isdigit(prev)) {
                        // checks the curr char is a char and prev char is a num
                        (*pixSize)++;
                        (*alphaSize)++;
                        if (DEBUG) {
                                printf("Current char is alpha, prev is int! Index at: %zu\n", i);
                        }      
                } else if (!isdigit((*currLine)[i])) {
                        // checks that curr is a char
                        (*alphaSize)++;
                        if (DEBUG) {
                                printf("Current char is alpha! Index at: %zu\n", i);
                        } 
                }
                // set prev to curr
                prev = (*currLine)[i]; 
        }
        
        // if last char is a number, increment pixValsSize by 1 to make
        // sure it gets counted
        if (isdigit((*currLine)[numBytes - 1])) {
                (*pixSize)++;
        }
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
        // create a cstring for injection patterns
        *pattern = malloc(*alphaSize + 1);

        size_t patternIndex = 0;

        // loop through entire line
        for (size_t i = 0; i < numBytes; i++) {
                if (!isdigit((*currLine)[i])) {
                        // the current char is an injection, add to string
                        (*pattern)[patternIndex] = (*currLine)[i];
                        patternIndex++;
                }
        }
        // add null terminator to the end of the line
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
void addToSeq(char** ogRow, Seq_T* ogRows, int height)
{
        Seq_put(&ogRows, height - 1, &ogRow); // height - 1 to convert to index

        if (DEBUG) {
                printf("Added line %s to index %d in Seq!\n", &ogRow, &height);
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
bool isOriginal(char** pattern, char** ogPattern, int pSize)
{       
        // check if they are the same length
        if (Atom_length(*ogPattern) != pSize) {
                return false;
        }

        for (int i = 0; i < pSize; i++) {
                if ((*pattern)[i] != (*ogPattern)[i]) {
                        // injected patterns not matching 
                        return false;
                }
        }

        return true;
}

/* validOrNot
 * 
 * This function checks if the pgm so far is a valid pgm (i.e., is there enough
 *      rows and columns, etc.)
 * 
 * Parameters:
 * */

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
void printPgm(Seq_T* ogRows, int width, int height)
{
                        
}


