#include "huffman_compression.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * List structs - you may define struct date_time and struct flight only. Each
 * struct definition should have only the fields mentioned in the assignment
 * description.
*******************************************************************************/


/*******************************************************************************
 * Function prototypes - do NOT change the given prototypes. However you may
 * define your own functions if required.
*******************************************************************************/
void printLoginMenu(void);
void printActionMenu(void);

/*******************************************************************************
 * Main
*******************************************************************************/
int main(void){

    printf("Hello World\n");

    compress_image_to_database("test.bmp");
    decompress_file_to_decompressed("test.bmp");
}

void printLoginMenu(void){
    printf("\n\n"
     "1. Login as Existing User\n"
     "2. Create a New User\n"
     "3. Exit\n");
}

void printActionMenu(void){
    printf("\n\n"
     "1. Save an Image from the Captures Folder\n"
     "2. Load an Image from the Database\n"
     "3. Remove an Image from the Database\n"
     "4. Exit the Program\n");
}