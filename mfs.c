/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: mfs.c
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/


// #include "mfs.h"
// #include "VCB.h"
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "DE.h"
// #include "fsLow.h"
//#include "fsInit.c"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "fsLow.h"
#include "mfs.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"

#define MAXDE 50


int fs_rmdir(const char *pathname){
    struct fdPathResult path = parsedPath(pathname);
    if (path.dirPtr == -1 && path.index == -1){
        return -1;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);

    //Gain access to the directory we want to remove by reading in its parent directory
    DirectoryEntry parentDir[MAXDE];
    LBAread(parentDir, dirBlocks, path.dirPtr);

    //Read in the directory we want to remove
    DirectoryEntry dirToRemove[MAXDE];
    LBAread(dirToRemove, dirBlocks, parentDir[path.index].location);
    
    //Loop through dirToRemove, checking that each DE except "." and ".." is known free state
    for (int i = 2; i < MAXDE; i++){
        if (strcmp(dirToRemove[i].name, "") != 0){
            return -1;
        }
    }

    //Mark blocks as free
    uint8_t* freeSpaceMap = malloc(getFreespaceSize(vcb->numBlocks, vcb->blockSize));
    LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    for (int i = parentDir[path.index].location; i < parentDir[path.index].location + dirBlocks; i++){
        setBitZero(freeSpaceMap, i);
    }

    //Set dirToRemove's DE to known free state
    strcpy(parentDir[path.index].name, "");

    //Write freespace and parentDir back to disk, free malloc
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
    LBAwrite(parentDir, dirBlocks, path.dirPtr);
    free(freeSpaceMap);
    freeSpaceMap = NULL;

    return 0;

}
int fs_delete(char* filename){
    struct fdPathResult path = parsedPath(filename);
    if (path.dirPtr == -1 && path.index == -1){
        return -1;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);
    
    //Gain access to the file we want to remove by reading in its parent directory
    DirectoryEntry parentDir[MAXDE];
    LBAread(parentDir, dirBlocks, path.dirPtr);

    //Get number of blocks being used by file
    int fileBlocks = (parentDir[path.index].size + (vcb->blockSize - 1)) / vcb->blockSize;

    //Mark blocks as free
    uint8_t* freeSpaceMap = malloc(getFreespaceSize(vcb->numBlocks, vcb->blockSize));
    LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    for (int i = parentDir[path.index].location; i < parentDir[path.index].location + fileBlocks; i++){
        setBitZero(freeSpaceMap, i);
    }

    //Set file's DE to known free state
    strcpy(parentDir[path.index].name, "");

    //Write freespace and parentDir back to disk, free malloc
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
    LBAwrite(parentDir, dirBlocks, path.dirPtr);
    free(freeSpaceMap);
    freeSpaceMap = NULL;

    return 0;
}

struct fdPathResult parsedPath(char * path){
    // check if absolute or relative
    char firstChar = path[0];
    int isAbsolute = 0;
    
    // this works
    if (strcmp(&firstChar, "/") == 0){
        isAbsolute = 1;
    }

    if (isAbsolute == 1){
        // make an array to load in data
        char * tokenArray[50];         // array of names to be tokenized
        const char s[2] = "/";         // delimiter
        int tokenIndex = 0;            // counter for number of tokens
        char str[strlen(path)];        // declare a string, str to be read of size strlen(path)
        strcpy(str,path);              // copy path into str


        // loop to tokenize values 
        char * token = strtok(str, s); 

        while (token != NULL){
            tokenArray[tokenIndex++] = token;
            //printf("token: %s\n", token);
            token = strtok(NULL, s);
        }

        // for (size_t i = 0; i < tokenIndex; i++)
        // {
        //     printf("tokenArray[i]: %s\n", tokenArray[i]);
        // }
        
        // do{
        //     //printf("token: %s\n", token);
        //     tokenArray[tokenIndex++] = token;
        // } while (token = strtok(NULL, "/"));



        //printf("tokenIndex: %d\n", tokenIndex);

        // load in root directory first
        // we know that its at location 6 

        // MAXDE requires include "fsinit.c" but multiple definitions
        DirectoryEntry *tempRoot = malloc(sizeof(DirectoryEntry) * MAXDE);
        
        // create a variable that changes for the loop to run
        int location = vcb->locOfRoot;

        struct fdPathResult result;
        
        // loop through all of the tokens
        for (size_t i = 0; i < tokenIndex; i++){
            LBAread(tempRoot, blocksNeededForDir(MAXDE), location);
            int j = 0;

            // loop through the directory entries for name comparison
            while (j < MAXDE){
                if (strcmp(tempRoot[j].name, tokenArray[i]) != 0){
                    location = tempRoot[j].location;

                    // this will update only once to grab the final
                    // index locaiton
                    if (i == tokenIndex - 1){
                        result.index = j;
                    }
                    break;
                }
                j++;
            }


            // find pointer to directory n-1
            // this will update multiple times but that's intentional
            if (i == tokenIndex - 2){
                result.dirPtr = tempRoot[i].location;
            }

            // in the case that we loop through the entire directory entries
            if (j == 50){
                printf("no directory with the name: %s\n", tokenArray[i]);
                result.dirPtr = -1;
                result.index = -1;
            }



            
            
        }
        return result;
        

        
        
    }

    
}

int fs_isFile(char * filename){

}	//return 1 if file, 0 otherwise



int fs_isDir(char * pathname);	//return 1 if directory, 0 otherwise


int fs_mkdir(const char *pathname, mode_t mode){
    
    int dirPtr = 30;
    int index = 5;
    int dirBlocks = blocksNeededForDir(MAXDE);

    // gain access to n directory by reading in the (n-1)directory
    DirectoryEntry parentDir[MAXDE];
    LBAread(parentDir, dirBlocks, dirPtr);

    // not 100 sure about this 
    // access directory at n
    DirectoryEntry dirToEnter[MAXDE];
    LBAread(dirToEnter, dirBlocks, parentDir[index].location);

    // loop through dirToEnter, checking each DE for an empty DE 
    for (size_t i = 2; i < MAXDE; i++){
        if (strcmp(dirToEnter[i].name, "") == 0){
            dirToEnter[i].fileType = FT_DIRECTORY;
        
        }
    }
    

    




}



