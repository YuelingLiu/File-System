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
#include <errno.h>
#include "fsLow.h"
#include "mfs.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"

#define MAXDE 50
#define MAXLENGTH 256

// int fs_rmdir(const char *pathname)
// {
//     struct fdPathResult path = parsedPath(pathname);
//     if (path.dirPtr == -1 && path.index == -1)
//     {
//         return -1;
//     }
//     int dirBlocks = blocksNeededForDir(MAXDE);

//     // Gain access to the directory we want to remove by reading in its parent directory
//     DirectoryEntry parentDir[MAXDE];
//     LBAread(parentDir, dirBlocks, path.dirPtr);

//     // Read in the directory we want to remove
//     DirectoryEntry dirToRemove[MAXDE];
//     LBAread(dirToRemove, dirBlocks, parentDir[path.index].location);

//     // Loop through dirToRemove, checking that each DE except "." and ".." is known free state
//     for (int i = 2; i < MAXDE; i++)
//     {
//         if (strcmp(dirToRemove[i].name, "") != 0)
//         {
//             return -1;
//         }
//     }

//     // Mark blocks as free
//     uint8_t *freeSpaceMap = malloc(getFreespaceSize(vcb->numBlocks, vcb->blockSize));
//     LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
//     for (int i = parentDir[path.index].location; i < parentDir[path.index].location + dirBlocks; i++)
//     {
//         setBitZero(freeSpaceMap, i);
//     }

//     // Set dirToRemove's DE to known free state
//     strcpy(parentDir[path.index].name, "");

//     // Write freespace and parentDir back to disk, free malloc
//     LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
//     LBAwrite(parentDir, dirBlocks, path.dirPtr);
//     free(freeSpaceMap);
//     freeSpaceMap = NULL;

//     return 0;
// }
// int fs_delete(char *filename)
// {
//     struct fdPathResult path = parsedPath(filename);
//     if (path.dirPtr == -1 && path.index == -1)
//     {
//         return -1;
//     }
//     int dirBlocks = blocksNeededForDir(MAXDE);

//     // Gain access to the file we want to remove by reading in its parent directory
//     DirectoryEntry parentDir[MAXDE];
//     LBAread(parentDir, dirBlocks, path.dirPtr);

//     // Get number of blocks being used by file
//     int fileBlocks = (parentDir[path.index].size + (vcb->blockSize - 1)) / vcb->blockSize;

//     // Mark blocks as free
//     uint8_t *freeSpaceMap = malloc(getFreespaceSize(vcb->numBlocks, vcb->blockSize));
//     LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
//     for (int i = parentDir[path.index].location; i < parentDir[path.index].location + fileBlocks; i++)
//     {
//         setBitZero(freeSpaceMap, i);
//     }

//     // Set file's DE to known free state
//     strcpy(parentDir[path.index].name, "");

//     // Write freespace and parentDir back to disk, free malloc

//     // do we have to change the fileType to FT_REGFILE?

//     //Write freespace and parentDir back to disk, free malloc

//     LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
//     LBAwrite(parentDir, dirBlocks, path.dirPtr);
//     free(freeSpaceMap);
//     freeSpaceMap = NULL;

//     return 0;
// }


struct fdPathResult parsedPath(const char * path){

    // check if absolute or relative
    char firstChar = path[0];
    int isAbsolute = 0;

    // this works
    if (strcmp(&firstChar, "/") == 0)
    {
        isAbsolute = 1;
    }

    if (isAbsolute == 1)
    {
        // make an array to load in data
        char *tokenArray[50];   // array of names to be tokenized
        const char s[2] = "/";  // delimiter
        int tokenIndex = 0;     // counter for number of tokens
        char str[strlen(path)]; // declare a string, str to be read of size strlen(path)
        strcpy(str, path);      // copy path into str

        // loop to tokenize values
        char *token = strtok(str, s);

        while (token != NULL)
        {
            tokenArray[tokenIndex++] = token;
            // printf("token: %s\n", token);
            token = strtok(NULL, s);
        }

        //printf("tokenIndex: %d\n", tokenIndex);
        

        // for (size_t i = 0; i < tokenIndex; i++)
        // {
        //     printf("tokenArray[i]: %s\n", tokenArray[i]);
        // }

        // do{
        //     //printf("token: %s\n", token);
        //     tokenArray[tokenIndex++] = token;
        // } while (token = strtok(NULL, "/"));

        // printf("tokenIndex: %d\n", tokenIndex);

        // load in root directory first
        // we know that its at location 6

        // MAXDE requires include "fsinit.c" but multiple definitions
        DirectoryEntry *tempRoot = malloc(sizeof(DirectoryEntry) * MAXDE);

        // create a variable that changes for the loop to run
        int location = vcb->locOfRoot;

        struct fdPathResult result;

        // assign the last value in tokenArray to result last arg
        // last arg may need to have a size initiated lastArg[20]
        strcpy(result.lastArg, tokenArray[tokenIndex]);
        

        // loop through all of the tokens
        for (size_t i = 0; i < tokenIndex; i++)
        {
            LBAread(tempRoot, blocksNeededForDir(MAXDE), location);
            int j = 0;

            // loop through the directory entries for name comparison
            while (j < MAXDE)
            {
                if (strcmp(tempRoot[j].name, tokenArray[i]) != 0)
                {
                    location = tempRoot[j].location;


                    // index location
                    if (i == tokenIndex - 1)
                    {

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
            if (j == 50)
            {
                printf("no directory with the name: %s\n", tokenArray[i]);
                result.dirPtr = -1;
                result.index = -1;
            }


            // find pointer to directory n-1
            if (i == tokenIndex - 2)
            {
                result.dirPtr = tempRoot[i].location;
            }

        }
        return result;
    }
}
}

// int fs_isFile(char *filename)
// {

// } // return 1 if file, 0 otherwise

// return 1 if directory, 0 otherwise
int fs_isDir(char *pathname){
}

int isFile(const char* name)
{
    struct fs_diriteminfo *ptr= fs_readdir(name);
    if(ptr != NULL)
    {
     closedir(ptr);
     ptr->fileType=FT_REGFILE;
     return 0;
    }

    if(errno == ENOTDIR)
    {
     return 1;
    }

    return -1;
}

    


// Misc directory functions
// This function is to get the working directory for the current task
// Returns a pointer to pathname, and pointer so NULL otherwise
// char *fs_getcwd(char *pathname, size_t size)
// {
//     // copy the abosulte pathname for the current working directory
//     printf("what is pathname %s\n", pathname);
//     char *cwd_buf;
//     if (pathname[0] != '/')
//     {
//         return NULL;
//     }

//     memcpy(cwd_buf, pathname, size);
//     if (strlen(pathname) + 1 >=size)
//     {
//         return NULL;
//     }

//     if (!cwd_buf)
//     {
//         cwd_buf = malloc(size);
//     }


//     strncpy(cwd_buf, pathname,size);
//     //printf("cwd_buf %s\n",cwd_buf);
//     return cwd_buf;
// }
// Linux chdir 

// int fs_setcwd(char *pathname){
//     // check if the pathname starts in the root direcotry 
//     if(pathname[0]!='/'){
//         return -1;
//     }
// }

// int fs_mkdir(const char *pathname, mode_t mode){
//     struct fdPathResult path = parsedPath(pathname);
//      if (path.dirPtr == -1 && path.index == -1){
//         return -1;
//     }
    
//     int dirBlocks = blocksNeededForDir(MAXDE);

//     // gain access to n directory by reading in the (n-1)directory
//     DirectoryEntry parentDir[MAXDE];
//     LBAread(parentDir, dirBlocks, path.dirPtr);

//     //Read in the directory we want to remove
//     DirectoryEntry dirToEnter[MAXDE];
//     LBAread(dirToEnter, dirBlocks, parentDir[path.index].location);


//     // loop through dirToEnter, checking each DE for an empty DE 
//     for (size_t i = 2; i < MAXDE; i++){
//         if (strcmp(dirToEnter[i].name, "") == 0){
//             dirToEnter[i].fileType = FT_DIRECTORY;
//             strcpy(dirToEnter[i].name, path.lastArg); // FIGURE THIS OUT
//         }
//     }
    
//     // mark blocks as used
//     uint8_t* freeSpaceMap = malloc(getFreespaceSize(vcb->numBlocks, vcb->blockSize));
//     LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
//     for (int i = parentDir[path.index].location; i < parentDir[path.index].location + dirBlocks; i++){
//         setBitOne(freeSpaceMap, i);
//     }

//     //Write freespace and parentDir back to disk, free malloc
//     LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
//     LBAwrite(parentDir, dirBlocks, path.dirPtr);
//     free(freeSpaceMap);
//     freeSpaceMap = NULL;

//     return 0;




// }

