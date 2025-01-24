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


/*  
int main(int argc, char *argv[])
{
        char* currline = nullptr;
        char* currPattern = nullptr;
        char* ogPattern = nullptr;
        size_t lineSize; // for holding return of readaline()

        set that holds non-numeric patterns of lines read, until we find the ogPattern
        sequence that holds the actual restored file
        table that holds key type char* (non-numeric patterns) and value type struct lineInfo
            -> table is used until we find the recurring non-numeric pattern
        
        while (not eof of file) 
        {
                lineSize = readaline(); // passed in currline, currline updated
                getPattern(); // extract non-numeric string 

                if (ogPattern is NULL)
                {
                        // we haven't found the pattern yet
                        if (addedToSet())
                        {
                                // the pattern is successfully added to the set, not recurring pattern
                                addtoTable();
                        } else {
                                // pattern already exists in set! We have found the ogPattern
                                ogPattern = currPattern;

                                // add current line to the sequence
                                addToSeq(currline);

                                // find the one original line in table by looking up the ogPattern
                                findLine();
                                // add manually to sequence before the current line
                        }
                } else {
                        // we have already found the pattern, horray! 
                        if (isOriginal())
                        {
                                // currline is an original line
                                addToSeq(currline);
                        } else {
                                // currline is an imposter!
                                // do nothing :)
                        }
                }
        }
        // we have now reached end of line
        // errorcheck and see if the pgm file we have right now is valid

        // valid, print out
        printPnm();
}
    
*/

