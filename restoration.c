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

void getLength(char **currLine, size_t numBytes, int *pixValsSize, 
                int *patternSize);
void getPattern(char **currLine, size_t numBytes, int *pixValsSize, 
                int *patternSize, char **pixVals, char **pattern);
// void printPgm(Seq_T* ogRows, int width, int height);

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

        char* currline = "";
        char* currPixVals = ""; // cstring to current pixel values
        char* currPattern = ""; // cstring to current injected chars
        //char* ogPattern = NULL; // cstring to pattern representing valid line
        size_t numBytes = 0; 

        int pixSize = 0; // size of pixel values 
        int alphaSize = 0; // size of injected value

        // set for finding the injectino pattern indicating an original line
        // Table_T uniqueInjs = Table_new(10, NULL, NULL);
        // sequence of pointers to char pointers
        // Seq_T ogRows = Seq_new(70);

        // retrieve data from file 
        // while (fgetc(inputFile) != EOF) 
        // {       
                // // read in a new line from the file
                // numBytes = readaline(inputFile, &currline);
                
                // // extract non-numeric string
                // getLength(&currline, numBytes, &pixSize, &alphaSize); 
                // getPattern(&currline, numBytes, &pixSize, &alphaSize, 
                //                 &currPixVals, &currPattern); 
                
        //         if (ogPattern == NULL)
        //         {
        //                 // we haven't found the injected pattern yet
        //         } else {
                        
        //         }

        //         /*
        //         if (ogPattern is NULL)
        //         {
        //                 // we haven't found the pattern yet
        //                 if (addedToSet())
        //                 {
        //                         // the pattern is successfully added to the set, not recurring pattern
        //                         addtoTable();
        //                 } else {
        //                         // pattern already exists in set! We have found the ogPattern
        //                         ogPattern = currPattern;

        //                         // add current line to the sequence
        //                         addToSeq(currline);

        //                         // find the one original line in table by looking up the ogPattern
        //                         findLine();
        //                         // add manually to sequence before the current line
        //                 }
        //         } else {
        //                 // we have already found the pattern, horray! 
        //                 if (isOriginal())
        //                 {
        //                         // currline is an original line
        //                         addToSeq(currline);
        //                 } else {
        //                         // currline is an imposter!
        //                         // do nothing :)
        //                 }
        //         }
        //         */
        // }             
        // we have now reached end of line
        // errorcheck and see if the pgm file we have right now is valid

        // valid, print out
        // printPnm();
        // Seq_free(&uniqueInjs); // free the sequence
        
        numBytes = readaline(inputFile, &currline);
        getLength(&currline, numBytes, &pixSize, &alphaSize);
        getPattern(&currline, numBytes, &pixSize, &alphaSize, 
                                &currPixVals, &currPattern); 
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
                        printf("Current char is alpha, prev is int! Index at: %zu\n", i);
                } else if (!isdigit((*currLine)[i])) {
                        // checks that curr is a char
                        (*alphaSize)++;
                        printf("Current char is alpha! Index at: %zu\n", i);
                }
                
                // set prev to curr
                prev = (*currLine)[i]; 
        }
        
        // if last char is a number, then increment pixValsSize by 1 to make
        // sure it gets counted
        if (isdigit((*currLine)[numBytes - 1])) {
                (*pixSize)++;
        }
}

/* getPattern
 * 
 * This function takes in a new line of the file and separates its pixel values
 *      from the injections 
 * 
 * Parameters: 
 *      char** currline:  A pointer to the cstring of current line read
 *      size_t numBytes:  The length of the line
 *      int* pixSize:     Pointer to int holding the length of pixel values in 
 *                            the line
 *      int* alphaSize:   Pointer to int holding the length of injected chars
 *      char** digitStr:  A pointer to the cstring holding pixel values in the 
 *                            line. A parameter to store result in & implicitly
 *                            return.
 *      char** pattern:   A pointer to the cstring holding injected chars in the 
 *                            line. A parameter to store result in & implicitly
 *                            return.  
 * 
 * Return: Nothing
 * 
 * Expects: pointers and Pointer-pointers to not be NULL
 * 
 * Note: This function  */
void getPattern(char **currLine, size_t numBytes, int *pixSize, 
                int *alphaSize, char **digitStr, char **pattern)
{
        *digitStr = malloc(*pixSize + 1);
        *pattern = malloc(*alphaSize + 1);

        char prev = 'b'; /*(*currLine)[0]*/
        size_t patternIndex = 0;
        size_t digitIndex = 0;

        // loop through entire line
        for (size_t i = 0; i < numBytes; i++) {
                if (!isdigit((*currLine)[i])) {
                // if curr is not a num, add it to pattern
                        (*pattern)[patternIndex] = (*currLine)[i];
                        patternIndex++;
                } else if (!isdigit(prev) && isdigit((*currLine)[i])) {
                // if prev is a char and curr is a num, add curr to pixVals
                        printf("Prev is a char, curr is a digit! Index: %zu\n", i);
                        (*digitStr)[digitIndex] = ((*currLine)[i]) - '0';
                        digitIndex++;
                } else if (isdigit(prev) && isdigit((*currLine)[i])) {
                // if prev and curr are nums, prev * 10 and add curr to prev
                        printf("At index: %zu, prev and curr are both digits!\n", i);
                        int pixV = (int)((*digitStr)[digitIndex - 1]) - '0';
                        printf("pixV1: %d\n", pixV);
                        pixV *= 10;
                        pixV += ((*currLine)[i]) - '0';
                        printf("pixV: %d\n", pixV);
                        (*digitStr)[digitIndex - 1] = pixV;
                        // (*pixVals)[pixValIndex - 1] *= 10; 
                        // (*pixVals)[pixValIndex - 1] += ((*currLine)[i]);
                }

                // set prev to curr
                prev = (*currLine)[i];
        }

        // add null terminator to the end of the line
        (*digitStr)[digitIndex] = '\0';
        (*pattern)[patternIndex] = '\0';

        printf("digitStr: %s\n", (*digitStr));
        printf("pattern: %s\n", (*pattern));
        printf("pixSize: %d\n", (*pixSize));
        printf("alphaSize: %d\n", (*alphaSize));
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
// void printPgm(Seq_T* ogRows, int width, int height)
// {
                        
// }


