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
struct fdPathResult globalTemp;
// DirectoryEntry *tempBuffer;
//fdDir *fd;
char globalPath[MAXLENGTH] = "/";
char finalPath[MAXLENGTH] = "/";
//struct fs_diriteminfo *retTempDir;

int fs_rmdir(const char *pathname)
{
    struct fdPathResult path = parsedPath(pathname);
    if (path.index == -1)
    {
        return -1;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);

    // Gain access to the directory we want to remove by reading in its parent directory
    DirectoryEntry parentDir[MAXDE];
    LBAread(parentDir, dirBlocks, path.dirPtr);

    // Read in the directory we want to remove
    DirectoryEntry dirToRemove[MAXDE];
    LBAread(dirToRemove, dirBlocks, parentDir[path.index].location);

    // Loop through dirToRemove, checking that each DE except "." and ".." is known free state
    for (int i = 2; i < MAXDE; i++)
    {
        if (strcmp(dirToRemove[i].name, "") != 0)
        {
            return -1;
        }
    }

    // Mark blocks as free
    //LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    for (int i = parentDir[path.index].location; i < parentDir[path.index].location + dirBlocks; i++)
    {
        setBitZero(freeSpaceMap, i);
    }

    // Set dirToRemove's DE to known free state
    strcpy(parentDir[path.index].name, "");
    printf("parentDir[path.index].name: %s\n", parentDir[path.index].name);

    // Write freespace and parentDir back to disk, free malloc
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
    LBAwrite(parentDir, dirBlocks, path.dirPtr);

    return 0;
}
//      int fs_delete(char *filename)
//      {
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

// used as a test function to populate storage for parse path to run
void testPopulateStorage(const char *path)
{
    // **************************************************
    //*********************************************
    // gotta figure this out
    printf("before\n");
    strcpy(globalPath, "/banana2");
    printf("after\n");
    /* TEST CODE */

    // LBAread and LBAwrite in storage so i have something to test and confirm the function works
    // EVERYTHING WORKS
    // 3 layers
    // Layer 1 ROOT
    // Layer 2 banana
    // layer 3 apple
    // layer 4 pear

    // move these two out into initfilesystem
    // fd = malloc(sizeof(fdDir));
    // retTempDir = malloc(sizeof(fs_diriteminfo));

    tempBuffer = malloc(sizeof(DirectoryEntry) * MAXDE);
    // remove volatile and test
    volatile int location = vcb->locOfRoot;
    volatile int numberofDE = MAXDE;

    // load in root
    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);

    // add in directory after ..
    // ***** we need to populate the rest of the data for the struct
    // ***** also need to mark free space map
    strcpy(tempBuffer[2].name, "banana");
    tempBuffer[2].fileType = FT_DIRECTORY;
    tempBuffer[2].numOfDE = 30;
    tempBuffer[2].location = 1000;

    // add another directory
    strcpy(tempBuffer[3].name, "banana2");
    tempBuffer[3].fileType = FT_DIRECTORY;
    tempBuffer[3].numOfDE = 40;
    tempBuffer[3].location = 2000;

    LBAwrite(tempBuffer, blocksNeededForDir(numberofDE), location);
    // testing
    LBAread(tempBuffer, 12, 0);
    // testing

    // success. I am able to create directories inside the root
    // root = . .. banana banana2
    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
    // printf("root: %s\n", tempBuffer[3].name);
    // printf("tempBuffer[3].numOfDE: %d\n", tempBuffer[3].numOfDE);

    // update values
    numberofDE = tempBuffer[3].numOfDE;
    location = tempBuffer[3].location;

    // read in banana2
    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);

    // fill in banana2
    // need to use directoryEntries because new folder does not have any inside
    // root already had directory entries populated
    DirectoryEntry directoryEntries[numberofDE];

    // set the dot
    strcpy(directoryEntries[0].name, ".");
    directoryEntries[0].location = location;
    directoryEntries[0].fileType = FT_DIRECTORY;

    // set the dot dot
    strcpy(directoryEntries[1].name, "..");
    directoryEntries[1].location = location;
    directoryEntries[1].fileType = FT_DIRECTORY;

    // set apple
    strcpy(directoryEntries[2].name, "apple");
    directoryEntries[2].location = location;
    directoryEntries[2].fileType = FT_DIRECTORY;
    directoryEntries[2].numOfDE = 55;
    directoryEntries[2].location = 3000;

    // set apple2
    strcpy(directoryEntries[3].name, "apple2");
    directoryEntries[3].location = location;
    directoryEntries[3].fileType = FT_DIRECTORY;
    directoryEntries[3].numOfDE = 66;
    directoryEntries[3].location = 4000;

    // set apple47
    strcpy(directoryEntries[25].name, "apple25");
    directoryEntries[25].location = location;
    directoryEntries[25].fileType = FT_DIRECTORY;

    LBAwrite(directoryEntries, blocksNeededForDir(numberofDE), location);

    // testing
    LBAread(tempBuffer, 12, 0);
    // printf("tempBuffer[0].name: %s\n", tempBuffer[0].name);

    // success it prints out . and ..
    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
    // printf("tempBuffer[0].name: %s\n", tempBuffer[2].name);

    // -> root -> level 1 need to go down one more layer
    // going into apple 2
    location = directoryEntries[3].location;
    numberofDE = directoryEntries[3].numOfDE;
    // apple 2 is at directoryEntries[3]
    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);

    // fill in pear2
    // need to use directoryEntries because new folder does not have any inside
    // root already had directory entries populated

    // set the dot
    strcpy(directoryEntries[0].name, ".");
    directoryEntries[0].location = location;
    directoryEntries[0].fileType = FT_DIRECTORY;

    // set the dot dot
    strcpy(directoryEntries[1].name, "..");
    directoryEntries[1].location = location;
    directoryEntries[1].fileType = FT_DIRECTORY;

    // set pear
    strcpy(directoryEntries[2].name, "pear");
    directoryEntries[2].location = location;
    directoryEntries[2].fileType = FT_DIRECTORY;
    directoryEntries[2].numOfDE = 77;
    directoryEntries[2].location = 5000;

    // set pear2
    strcpy(directoryEntries[3].name, "pear2");
    directoryEntries[3].location = location;
    directoryEntries[3].fileType = FT_REGFILE;
    directoryEntries[3].numOfDE = 88;
    directoryEntries[3].location = 6000;

    LBAwrite(directoryEntries, blocksNeededForDir(numberofDE), location);

    LBAread(tempBuffer, 12, 0);
    // printf("tlevel 2: %s\n", tempBuffer[0].name);

    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
    // printf("tempBuffer[0].name: %s\n", tempBuffer[3].name);

    /* TEST CODE */
}

struct fdPathResult parsedPath(const char *path)
{
    printf("0*****************************************\n");
    // check if absolute or relative
    
    printf("checking this path %s\n", path);
    char firstChar = path[0];
    int isAbsolute = 0;
    struct fdPathResult result;
    // confirm if the path is relative to absolute
    if (firstChar == '/')
    {
        isAbsolute = 1;
        // in the case that the path is just the root
        // if (strlen(path) == 1){
        //     result.dirPtr = vcb->locOfRoot;

        //     int size = strlen(path);
        //     strcpy(result.lastArg, fs_getcwd(path,size));

        //     // retrieve result.index

        // }
    }

    // for a relative path, we need to grab the path first
    /*
    relative path means you want to find the file/folder in the current directory
    first we need to get current directory (absolute path)
    then we need to tokenize the relative path. to grab the last argument
    we then concatenate the current directory with the absolute path
    once we have the entire absolute path, we run parsepath and this time,
    absolute value is 1 so it parses the entire path and returns fdPathResult
    */
    if (isAbsolute == 0)
    {
        // grab in global current directory variable which is the absolute location
        // char * currentDir = fs_getcwd(path,strlen(path));
        char currentDir[4096];
        strncpy(currentDir, globalPath, 4096);

        // from root given(root location)
        // parse the absolute location
        // starting from root, we go to next location, then next location
        // until we get to end then finally we search for path.name

        // this is an edge case such that if the currentDir is "/"
        // we dont want to add another "/" as that would set the
        // currentDir to "//"
        // so if the currdir is "/", we skip concatenating "/"
        // if the currDir is "/banana", we concatenate "/"
        printf("currentDir: %s\n", currentDir);
        if (strcmp(currentDir, "/") != 0 && strlen(currentDir) > 2)
        {
            strcat(currentDir, "/");
        }
        strcat(currentDir, path);
        // printf("checking path %s\n",path);

        // printf("After currentDir: %s\n", currentDir);

        struct fdPathResult tempPath = parsedPath(currentDir);

        result.dirPtr = tempPath.dirPtr;
        result.index = tempPath.index;
        strcpy(result.lastArg, tempPath.lastArg);
        return result;
    }

    // for absolute path, we will parse the path starting from root
    if (isAbsolute == 1)
    {
        // tokenizes path into tokens
        char *tokenArray[50];     // array of names to be tokenized
        char *finalPathArray[50]; // for edge cases of . and ..
        const char s[2] = "/";    // delimiter
        int tokenIndex = 0;       // counter for number of tokens
        char str[strlen(path)];   // declare a string, str to be read of size strlen(path)
        strcpy(str, path);        // copy path into str

        // loop to tokenize values
        char *token = strtok(str, s);

        // testing function
        
        while (token != NULL)
        {
            tokenArray[tokenIndex++] = token;
            token = strtok(NULL, s);
        }

        for (size_t i = 0; i < tokenIndex; i++)
        {
            printf("tokenIndex: %s\n", tokenArray[i]);
        }
        
        // in the case that user enters in a weird input
            // ie /./../././banana2
        
        int counterBegin = tokenIndex -1;
        int counter = counterBegin;
        int arrayCounter = 0;
        bool dotFlag = 0;

        // printf("globalPath: %s\n", globalPath);
        // printf("transfering to finalPathArray\n");

        printf("path: %s\n", path);


        // for (size_t i = 0; i < tokenIndex; i++)
        // {
        //     if ((strcmp(tokenArray[i], ".") == 0))
        //     {
        //         dotFlag = 1;
        //     }
        //     else if ((strcmp(tokenArray[i], "..") == 0))
        //     {   
        //         // reduce counter so we can remove item in array
        //         dotFlag = 1;
        //         finalPathArray[counter] = "";
        //         counter--;    
        //     }
        //     else
        //     {   
        //         // add it to temp array
        //         finalPathArray[counter] = tokenArray[i];
        //         counter++;
        //     }
        // }
        
            

        

        // /banana/banana/banana
        // strcpy(finalPath, "");
        // printf("after finalPath asdfasd""\n");
        
        // for (size_t i = counterBegin; i < counter; i++)
        // {
        //     strcat(finalPath, "/");
        //     strcat(finalPath, finalPathArray[i]);
        // }
        // printf("finalPath: %s\n", finalPath);

        // print tokens

        // while (token = strtok(NULL, "/"))
        // {
        //     tokenArray[tokenIndex++] = token;
        // }

        // printf("counter: %d\n", counter);

        //******************************************
        // strcpy(globalPath, finalPath);
        // printf("globalPath after: %s\n", globalPath);
        // // /banana/./../banana2/./apple2/../apple3
        // printf("****************************\n");

        // end of tokenizer


        // /banana/banana/banana
        // strcpy(finalPath, "");
        // printf("after finalPath asdfasd""\n");
        
        // for (size_t i = counterBegin; i < counter; i++)
        // {
        //     strcat(finalPath, "/");
        //     strcat(finalPath, finalPathArray[i]);
        // }
        // printf("finalPath: %s\n", finalPath);

        // print tokens

        // while (token = strtok(NULL, "/"))
        // {
        //     tokenArray[tokenIndex++] = token;
        // }

        // printf("counter: %d\n", counter);

        //******************************************
        // strcpy(globalPath, finalPath);
        // printf("globalPath after: %s\n", globalPath);
        // // /banana/./../banana2/./apple2/../apple3
        // printf("****************************\n");

        // end of tokenizer

        // im using tempBuffer instead of tempRoot
        // create a variable that changes for the loop to run
        // commented out bc location is made in the test above
        printf("2*****************************************\n");
        int location = vcb->locOfRoot;

        // assign the last value in tokenArray to result last arg
        // save last arg
        printf("3*****************************************\n");
        printf("tokenArray[tokenIndex -1]: %s\n", tokenArray[tokenIndex]);
        printf("tokenIndex: %d\n", tokenIndex);
        if (tokenIndex > 0){
            strcpy(globalTemp.lastArg, tokenArray[tokenIndex - 1]);
        }
        printf("4*****************************************\n");

        int numberofDE = MAXDE;
        // loop through all of the tokens in root
        for (size_t i = 0; i < tokenIndex; i++)
        {
            printf("5*****************************************\n");
            LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
            printf("6*****************************************\n");
            int j = 0;

            // loop through the directory entries for name comparison
            while (j < numberofDE)
            {
                if (strcmp(tempBuffer[j].name, tokenArray[i]) == 0)
                {
                    // works up to here 100%

                    // this will update and it's to grab the final index
                    // index locaiton
                    location = tempBuffer[j].location;
                    // printf("location: %d\n", location);
                    numberofDE = tempBuffer[j].numOfDE;
                    
                    // printf("numberofDE: %d\n", numberofDE);
                    // printf("tempBuffer[j].name22: %s\n", tempBuffer[j].name);

                    // once the comparison is found, we read in the next location
                    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
                    break;
                }
                j++;
                // grabbing result
                if (i == tokenIndex - 1)
                {
                    globalTemp.index = j;
                    globalTemp.dirPtr = tempBuffer[i].location;
                    
                    // printf("result.index: %d\n", result.index);
                    // break;
                }

                // in the case that we loop through the entire directory entries
                if (j == numberofDE - 1)
                {
                    printf("no directory with the name: %s\n", tokenArray[i]);
                    if (i > 0)
                    {
                        globalTemp.dirPtr = tempBuffer[i - 1].location;
                    }
                    else
                        globalTemp.dirPtr = tempBuffer[0].location;
                    globalTemp.index = -1;
                    result.dirPtr = globalTemp.dirPtr;
                    result.index = globalTemp.index;
                    strcpy(result.lastArg, globalTemp.lastArg);
                    return result;
                    // break;
                }

            }
            // prints out 1 bc the first parameter is /./notbanana
            // printf("j: %d\n", j);

            // find pointer to directory n-1
            // this will update multiple times but that's intentional
            // this works
            // if (i == tokenIndex - 2)
            // {
            //     globalTemp.dirPtr = tempBuffer[i].location;
            // }
        }
        

        result.dirPtr = globalTemp.dirPtr;
        result.index = globalTemp.index;
        strcpy(result.lastArg, globalTemp.lastArg);

        printf("about to free inside parsepath\n");
        
        // tempBuffer = NULL;
        // free(tempBuffer);

        return result;
    }
    // these arent ran bc after return
    //printf("about to free inside parsepath\n");
    // free(tempBuffer);
    // tempBuffer = NULL;
}

int fs_isFile(char *filename)
{
    // run parsepath to get a struct
    // so the tempLastArg is the current folder

    // parsepath will determine if its in the same folder or absolute

    struct fdPathResult tempPath = parsedPath(filename);

    LBAread(tempBuffer, MAXDE, tempPath.dirPtr);

    for (size_t i = 0; i < MAXDE; i++)
    {
        if (strcmp(tempBuffer[i].name, tempPath.lastArg) == 0)
        {
            if (tempBuffer[i].fileType == FT_REGFILE)
            {
                return 1;
            }
        }
    }

    return 0;
}

int fs_isDir(char *pathname)
{
    // run parsepath we get a struct to confirm it is a path as welll as location and stuff
    struct fdPathResult tempPath = parsedPath(pathname);
    // dirPtr, index, lastArg
    // LBAread (tempBuffer, MAXDE, dirPtr);
    LBAread(tempBuffer, MAXDE, tempPath.dirPtr);

    // iterate through tempBuffer to strcpy(tempbuffer[i].name, lastArg) == 0
    for (size_t i = 0; i < MAXDE; i++)
    {
        // check in the same folder and see if the file name passed in exists in the
        // current directory.
        if (strcmp(tempBuffer[i].name, tempPath.lastArg) == 0)
        {
            if (tempBuffer[i].fileType == FT_DIRECTORY)
            {
                return 1;
            }
        }
        // in the case that i is MAXDE and tempBuffer[i].name does not exist
        // return 0
        if (i == MAXDE - 1)
        {
            printf("file does not exist***\n");
            return 0;
        }
    }
    printf("file does not exist\n");
    return 0;
}

// Misc directory functions
// This function is to get the working directory for the current task
// Returns a pointer to pathname, and pointer so NULL otherwise

// int fs_mkdir(const char *pathname, mode_t mode){
//     struct fdPathResult path = parsedPath(pathname);
//      if (path.dirPtr == -1 && path.index == -1){
//         return -1;
//     }

// char *fs_getcwd(char *pathname, size_t size)
// {
//     // copy the abosulte pathname for the current working directory
//     printf("what is pathname %s\n", pathname);
//     char *cwd_buf;
//     if (pathname[0] != '/')
//     {
//         return NULL;
//     }

// Returns a pointer to absolute pathname, and pointer so NULL otherwise

// getcwd version 2
char *fs_getcwd(char *pathname, size_t size)
{
    // why does it need these parameters?
    // if (strlen(globalPath) > size){
    //     return NULL;
    // }
    strncpy(pathname, globalPath, size);
    return globalPath;
}

// Linux chdir

//  /banana/./../banana2/./apple2/../apple3
int fs_setcwd(char *pathname)
{   
    struct fdPathResult path = parsedPath(pathname);
    printf("///////////////////////////////\n");
    printf("inside setcwd after parsedPath\n");

    if (path.index == -1)
    {
        printf("path.index is -1 \n");
        return -1;
    }
    else
    {   
        // case that global path is just "/"
        if(strlen(globalPath) == 1){
            // concatenate to globalPath
            strcat(globalPath, pathname);
            // copy concatenated global path to pathname buffer
            strcpy(pathname, globalPath);
        } 
        else {
            // same as above except if the global path starts with root
            strcat(globalPath, "/");
            strcat(globalPath, pathname);
            strcpy(pathname, globalPath);
        }  
        printf("inside setcwd before return 0\n");
        return 0;
    }

    // // If not absolute path, must append paramter to CWD
    //  if(pathname[0]!='/'){
    //      //If cwd IS NOT root and cwd doesn't already end in slash, append slash first
    //      if (strcmp(globalPath, "/") != 0){
    //      }
    //      //strcat(globalPath, pathname);
    //      //If last char is a slash, get rid of it
    //      if (globalPath[strlen(globalPath)-1] == '/'){
    //          globalPath[strlen(globalPath)-1] = '\0';
    //      }

    //     return 0;
    // }
    // //For absolute path, set parameter as CWD
    // else {
    //     // **************************************************
    //     //strcpy(globalPath, pathname);
    //     //If last char is a slash, get rid of it
    //     if (globalPath[strlen(globalPath)-1] == '/'){
    //         globalPath[strlen(globalPath)-1] = '\0';
    //     }
    //     return 0;
    // }
}

// swtcwd version 2
// int fs_setcwd(char *pathname){

//     strcpy(globalPath, pathname);

//     return 0;

//     // return a different number if error

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

// mkdir version 2
int fs_mkdir(const char *pathname, mode_t mode)
{
    /*
    1 grab current directory
    2 parsePath
    3 LBAread
    4 iterate through to find name empty
    5 set as folder
    6 find space in free space map
    7 populate space with DEs
    8 return 1
    */
    struct fdPathResult path = parsedPath(pathname);
    // if the file already exists we dont need to make another
    if (path.index != -1)
    {
        printf("file exists\n");
        return -1;
    }
    printf("path.index: %d\n", path.index);
    printf("path.dirPtr: %d\n", path.dirPtr);
    printf("path.lastArg: %s\n", path.lastArg);
    // int tempSize = 10;
    // char * tempCurrentDir = fs_getcwd(pathname ,tempSize);

    // int dirBlocks = blocksNeededForDir(MAXDE);
    // //calling parsepath on cwd returns dirPtr to PARENT of cwd, plus cwd's index
    // //so we need to read PARENT of cwd first

    // we have the pointer to the directory of where we want
    // to make the path.lastArg
    printf("inside mkdir*****************************************\n");
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), path.dirPtr);

    // DirectoryEntry parentOfCurrentDir[MAXDE];
    // LBAread(parentOfCurrentDir, dirBlocks, path.dirPtr);
    // //now we can read cwd
    // DirectoryEntry currentDir[MAXDE];
    // LBAread(currentDir, dirBlocks, parentOfCurrentDir[path.index].location);

    int i = 2; // starting dir index of NOT "." or ".."
    while (i < MAXDE)
    {
        if (strcmp(tempBuffer[i].name, "") == 0)
        { // Upon finding first available DE slot
            // Prepare freespace
            size_t fssize = getFreespaceSize(vcb->numBlocks, vcb->blockSize);
            // uint8_t *freeSpaceMap = malloc(fssize);
            //LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
            int locOfNewDir = allocContBlocks(freeSpaceMap, fssize, blocksNeededForDir(MAXDE));

            // Prepare DE of new directory
            strcpy(tempBuffer[i].name, path.lastArg);
            tempBuffer[i].size = MAXDE * sizeof(DirectoryEntry);
            tempBuffer[i].fileType = FT_DIRECTORY;
            tempBuffer[i].numOfDE = MAXDE;
            tempBuffer[i].location = locOfNewDir;
        printf("tempBuffer[%d].name: %s\n", i,tempBuffer[i].name);

            // Prepare the new directory itself
            printf("inside mkdir before newDir*****************************************\n");

            DirectoryEntry newDir[MAXDE];                     

            // initialize each directory entry of NEW DIR to be in a known free state
            for (int j = 0; j < MAXDE; j++)
            {
                strcpy(newDir[j].name, "");
            }
            // set the dot
            strcpy(newDir[0].name, ".");
            newDir[0].fileType = FT_DIRECTORY;
            newDir[0].location = locOfNewDir;
            printf("after strcpy\n");
            // set the dot dot
            strcpy(newDir[1].name, "..");
            newDir[1].fileType = FT_DIRECTORY;
            newDir[1].location = path.dirPtr;
            printf("afterstrcpy2\n");

            LBAwrite(tempBuffer, blocksNeededForDir(MAXDE), path.dirPtr);
            LBAwrite(newDir, blocksNeededForDir(MAXDE), locOfNewDir);
            int writeReturn;
            printf("after LBAwrite\n");
            if (writeReturn = LBAwrite(freeSpaceMap, 5, 1) != 5){
                printf("Error Writing with LBAwrite, exiting program\n");
                exit(-1);
            }
            printf("before locofNEWdir\n");
            printf("locOfNewDir: %d\n", locOfNewDir);
            // return locOfNewDir;
            return 0;
        }
        i++;
    }
    printf("Cannot make new dir, parent dir is full\n");
    return -1;
}

//
fdDir *fs_opendir(const char *pathname)
{
    printf("works inside of opendir\n");
    // 1. parse the pathname, make sure path is valid and find the last element
    struct fdPathResult tempPath = parsedPath(pathname);

    // 2. check the last element to see if it is a directory
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), tempPath.dirPtr);
    // printf("tempBuffer[tempPath.index].fileType: %d\n", tempBuffer[tempPath.index].fileType);

    //      a: yes if last Arg type  IS directory
    //      b: no -> fail return null it is not a directory
    if (tempBuffer[tempPath.index].fileType != FT_DIRECTORY)
    {
        printf("return NULL\n");
        return NULL;
    }

    // 3.  Load this directory
    
    // load directory into starting location for LBAread
    fdDir* fd = malloc(sizeof(fdDir));
    fd->directoryStartLocation = tempBuffer[tempPath.index].location;
    

    printf("fd->directoryStartLocation: %ld\n", fd->directoryStartLocation);

    
    fd->dirEntryPosition = 0;
    fd->dirp_fs = malloc(sizeof(fs_diriteminfo));
    return fd;
}

// takes a pointer and returns a pointer to fs_diriteminfo struct
// fs_diriteminfo contains d_name, filetType, d_reclen
// this is just do a for loop
// returns a pointer to fs_diriteminfo struct
// return null when Error occurs

fs_diriteminfo *fs_readdir(fdDir *fd){
   // start from where we last left off, which was position 0

   LBAread(tempBuffer, blocksNeededForDir(MAXDE), fd->directoryStartLocation);

    for(int i = fd->dirEntryPosition ; i < MAXDE; i++){
        //if this directory is used,
        //if DirectoryEntryUsed(dirp->dirp[i]){
        if (strcmp(tempBuffer[i].name, "") != 0){

            // ii fs_diriteminfo
            // copy the name from our directory entry to the struct
            // NEED TO FIX THIS
            strcpy(fd->dirp_fs->d_name, tempBuffer[i].name);

            //printf("retTempDir->d_name: %s\n", retTempDir->d_name);
            // copy the fileType over to struct
            fd->dirp_fs->fileType = tempBuffer[i].fileType;

            // iterate the directory entry position to read the next slot
            fd->dirEntryPosition = i+1;

            //return fd->dirp_fs;
            return fd->dirp_fs;

        }
    }
    return NULL;

}

/* open dir opens up a folder for you to iterate through
we have to use the directory entry value of dirPtr inside of
fs_diriteminfo so we can LBAread into that directory entry
so readdir can iterate through that directory entry
*/
// my version
// struct fs_diriteminfo *fs_readdir(fdDir *dirp){
//     //base case
//     if(dirp ==NULL){
//         return NULL;
//     }

//     //LBAread(tempBuffer, MAXDE, ptrToDir)

//     for(int i = 0 ; i < MAXDE;i++){
//         // DirectoryEntryUsed by checking the d_reclen length or dirEntryPosition
//         if(dirp[i].d_reclen > 4){
//              // copy the name from our directory entry to the struct
//             strcpy(dirp->dirp_fs->d_name,dirp[i].d_name);
//             dirp->dirp_fs->fileType = FT_DIRECTORY;
//             dirp->dirEntryPosition =i+1;

//           return dirp->dirp_fs;
//     }
//     }
//     return NULL;

// }

// free up memory here  we allocated in the open
int fs_closedir(fdDir *fd)
{
    free(fd->dirp_fs);
    free(fd);
    fd = NULL;
}

int fs_stat(const char *path, struct fs_stat *buf){
    struct fdPathResult result = parsedPath(path);
    if (result.index == -1){
        return -1;
    }
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), result.dirPtr);
    buf->st_size = tempBuffer[result.index].size;
    buf->st_blocks = (tempBuffer[result.index].size + (vcb->blockSize - 1))/vcb->blockSize;
    return 0;
}
