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
    DirectoryEntry * parentDir = calloc (dirBlocks, vcb->blockSize);
    printf("parentDir: %p\n", parentDir);
    printf("tempBuffer: %p\n", tempBuffer);
    LBAread(parentDir, dirBlocks, path.dirPtr);
    
    // Read in the directory we want to remove
    DirectoryEntry * dirToRemove = calloc (dirBlocks, vcb->blockSize);
    LBAread(dirToRemove, dirBlocks, parentDir[path.index].location);
    
    // Loop through dirToRemove, checking that each DE except "." and ".." is known free state
    for (int i = 2; i < MAXDE; i++)
    {
        if (strcmp(dirToRemove[i].name, "") != 0)
        {
            printf("can not remove directory because it is not empty.\n");
            printf("***************dirToRemove[i].name: %s\n", dirToRemove[i].name);
            return -1;
        }
    }

    // Mark blocks as free
    LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    for (int i = parentDir[path.index].location; i < parentDir[path.index].location + dirBlocks; i++)
    {
        setBitZero(freeSpaceMap, i);
    }

    // Set dirToRemove's DE to known free state
    strcpy(parentDir[path.index].name, "");
    parentDir[path.index].size = 0;
    parentDir[path.index].location = 0;
    parentDir[path.index].fileType = 0;
    parentDir[path.index].numOfDE = 0;
    printf("parentDir[path.index].name: %s\n", parentDir[path.index].name);

    // Write freespace and parentDir back to disk, free malloc
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
    LBAwrite(parentDir, dirBlocks, path.dirPtr);

    free(parentDir);
    free(dirToRemove);

    return 0;
}

int fs_delete(char *filename)
     {
    struct fdPathResult path = parsedPath(filename);
    if (path.dirPtr == -1 && path.index == -1)
    {
        return -1;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);

    // Gain access to the file we want to remove by reading in its parent directory
    DirectoryEntry * parentDir = calloc (dirBlocks, vcb->blockSize);
    LBAread(parentDir, dirBlocks, path.dirPtr);

    // Get number of blocks being used by file
    int fileBlocks = (parentDir[path.index].size + (vcb->blockSize - 1)) / vcb->blockSize;

    // Mark blocks as free
    for (int i = parentDir[path.index].location; i < parentDir[path.index].location + fileBlocks; i++)
    {
        setBitZero(freeSpaceMap, i);
    }

    // Set file's DE to known free state
    strcpy(parentDir[path.index].name, "");

    // Write freespace and parentDir back to disk

    // do we have to change the fileType to FT_REGFILE?

    //Write freespace and parentDir back to disk

    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
    LBAwrite(parentDir, dirBlocks, path.dirPtr);

    free(parentDir);
    return 0;
}

//*parse
struct fdPathResult parsedPath(const char *path)
{
    // check if absolute or relative
    
    printf("checking this path %s\n", path);
    
    char firstChar = path[0];
    // tempBuffer = malloc(sizeof(DirectoryEntry) * MAXDE);
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
        
        const char s[2] = "/";    // delimiter
        int tokenIndex = 0;       // counter for number of tokens
        char str[strlen(path)];   // declare a string, str to be read of size strlen(path)
        strcpy(str, path);        // copy path into str
        
        //tempBuffer = malloc(sizeof(DirectoryEntry) * MAXDE);

        // loop to tokenize values
        char *token = strtok(str, s);

        // tokenizes the values and inserts into tokenArray
        //tokenArray[0] = "/";
        while (token != NULL)
        {
            tokenArray[tokenIndex++] = token;
            token = strtok(NULL, s);
        }

        
            

        
        // create a variable that changes for the loop to run
        // commented out bc location is made in the test above
        int location = vcb->locOfRoot;

        // assign the last value in tokenArray to result last arg
        // save last arg

        printf("tokenIndex: %d\n", tokenIndex);
        if (tokenIndex > 0){
            strcpy(globalTemp.lastArg, tokenArray[tokenIndex - 1]);
        }

        // edge case of just root inputted
        int numberofDE = MAXDE;
        
        // *loop
        // loop through all of the tokens in root
        printf("before big loop in parsepath\n");
        for (int i = 0; i < tokenIndex; i++)
        {
            printf("starting for loop iteration i = %d\n", i);
            LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
            
            int j = 0;

            // loop through the directory entries for name comparison
            while (j < numberofDE)
            { 
                printf("starting while loop iteration j = %d\n", j);
                if (strcmp(tempBuffer[j].name, tokenArray[i]) == 0)
                {
                    // works up to here 100%

                    // this will update and it's to grab the final index
                    // index locaiton
                    printf("printing inside loop\n");
                    printf("tempBuffer[j].name: %s\n", tempBuffer[j].name);
                    printf("tokenArray[i]: %s\n", tokenArray[i]);
                
                    location = tempBuffer[j].location;
                    globalTemp.dirPtr = tempBuffer[0].location;
                    // printf("location: %d\n", location);
                    numberofDE = tempBuffer[j].numOfDE;

                    printf("location: %d\n", location);
                    printf("numberofDE: %d\n", numberofDE);
                    printf("end of printing inside loop ****\n");

                    // once the comparison is found, we read in the next location
                    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
                    // free(tempBuffer);
                    // tempBuffer = NULL;
                    break;
                }
                j++;
                // grabbing result
                if (i == tokenIndex - 1) //This executes on final token
                {
                    printf("grabbing result, setting globalTempt index to %d\n", j);
                    globalTemp.index = j;
                    //globalTemp.dirPtr = tempBuffer[i].location;
                    
                    // printf("result.index: %d\n", result.index);
                    //break;
                }

                // in the case that we loop through the entire directory entries
                if (j == numberofDE - 1)
                {
                    printf("no directory with the name: %s\n", tokenArray[i]);
                    if (i > 0)
                    {
                       // printf("setting dirPtr to tempBuffer[0].location: %d\n",tempBuffer[0].location);
                        globalTemp.dirPtr = tempBuffer[0].location;
                    }
                    else{
                    // This condition triggers only when directory is missing AT ROOT
                        
                        printf("setting dirPtr to locOfRoot\n");
                        globalTemp.dirPtr = vcb->locOfRoot;
                    }
                    globalTemp.index = -1;
                    result.dirPtr = globalTemp.dirPtr;
                    result.index = globalTemp.index;
                    strcpy(result.lastArg, globalTemp.lastArg);
                    
                    
                    /* TEST CODE */
                    // if(tokenFlag == 1){
                    //     printf("globalPath**************: %s\n", globalPath);
                    //     strcpy(globalPath, finalPath);
                    //     printf("globalPath**************: %s\n", globalPath);
                    // }



                    /* TEST CODE */
                    printf("returning from for loop\n");
                    printf("result.dirPtr: %d\n", result.dirPtr);
                    printf("result.index: %d\n", result.index);
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
        
        //This executes when result is found
        result.dirPtr = globalTemp.dirPtr;
        result.index = globalTemp.index;
        strcpy(result.lastArg, globalTemp.lastArg);

        if (location == vcb->locOfRoot){
            result.dirPtr = vcb->locOfRoot;
            result.index = 0;
            strcpy(result.lastArg, "/");
        }
        printf("returning outside for loop\n");
        printf("result.dirPtr: %d\n", result.dirPtr);
        printf("result.index: %d\n", result.index);
        
        /* TEST CODE */
        // if(tokenFlag == 1){
        //     printf("globalPath**************: %s\n", globalPath);
        //     strcpy(globalPath, finalPath);
        //     printf("globalPath**************: %s\n", globalPath);
        // }


        /* TEST CODE */
        

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
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), tempPath.dirPtr);

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
    printf("pathname inside of getcwd: %s\n", pathname);
    printf("globalPath inside of getcwd: %s\n", globalPath);
    // this causes a stack smashing detection crash
    strncpy(pathname, globalPath, size);
    return globalPath;
}

// Linux chdir
void fs_pathReconstruction (){
    // tokenizes path into tokens
    char *tokenArray[50];     // array of names to be tokenized
    const char s[2] = "/";    // delimiter
    int tokenIndex = 0;       // counter for number of tokens
    char str[strlen(globalPath)];   // declare a string, str to be read of size strlen(path)
    strcpy(str, globalPath);        // copy path into str
    
    // banana/apple/pear/../../apple

    // loop to tokenize values
    char *token = strtok(str, s);

    // tokenizes the values and inserts into tokenArray
    //tokenArray[0] = "/";
    while (token != NULL)
    {
        tokenArray[tokenIndex++] = token;
        token = strtok(NULL, s);
    }



    char finalPath[MAXLENGTH] = "";
    char *finalPathArray[50]; // for edge cases of . and ..
    bool tokenFlag = 0;       // flag for . and ..
    //struct fdPathResult path = parsedPath(pathname);

    /* TEST CODE */
    // token printer
    for (size_t i = 0; i < tokenIndex; i++)
    {
        printf("tokenArray[i]***********: %s\n", tokenArray[i]);
        if ( (strcmp(tokenArray[i],".") == 0)  || (strcmp(tokenArray[i],"..") == 0)){
            tokenFlag = 1;
        }
    }
    printf("tokenIndex: %d\n", tokenIndex);
    printf("tokenArray[0]: %s\n", tokenArray[0]);
    // if there are . or .. we want to remove that from the global path
    if(tokenFlag == 1 && tokenIndex > 2){
        int counter = 0;
        for (size_t i = 0; i < tokenIndex; i++)
        {
        if ((strcmp(tokenArray[i], ".") == 0))
        {
            // do nothing
        }
        else if ((strcmp(tokenArray[i], "..") == 0))
        {   
            // reduce counter so we can remove item in array
            finalPathArray[counter] = "";
            counter--;    
        }
        else
        {   
            // add it to temp array
            finalPathArray[counter] = tokenArray[i];
            counter++;
        }
        }
        
        for (size_t i = 0; i < counter; i++){
            strcat(finalPath, "/");
            strcat(finalPath, finalPathArray[i]);
        }
        
        printf("finalPath************: %s\n", finalPath);
        
        strcpy(globalPath, finalPath);
                
        //tokenFlag = 0;

    }
}
 // banana/apple/pear/../../apple
int fs_setcwd(char *pathname)
{   
    struct fdPathResult path = parsedPath(pathname);
    printf("globalPath******(): %s\n", globalPath);
    //fs_pathReconstruction(pathname);

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
        fs_pathReconstruction();
        

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
// makedir
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
    //tempBuffer = malloc(sizeof(DirectoryEntry) * MAXDE);

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
            printf("locOfNewDir: %d\n", locOfNewDir);

            // Prepare DE of new directory
            strcpy(tempBuffer[i].name, path.lastArg);
            tempBuffer[i].size = MAXDE * sizeof(DirectoryEntry);
            tempBuffer[i].fileType = FT_DIRECTORY;
            tempBuffer[i].numOfDE = MAXDE;
            tempBuffer[i].location = locOfNewDir;
            printf("tempBuffer[i].location: %ld\n", tempBuffer[i].location);
        printf("tempBuffer[%d].name: %s\n", i,tempBuffer[i].name);

            // Prepare the new directory itself

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
            newDir[0].numOfDE = MAXDE;
            // set the dot dot
            strcpy(newDir[1].name, "..");
            newDir[1].fileType = FT_DIRECTORY;
            newDir[1].location = path.dirPtr;
            newDir[1].numOfDE = tempBuffer[0].numOfDE;

            LBAwrite(tempBuffer, blocksNeededForDir(MAXDE), path.dirPtr);
            LBAwrite(newDir, blocksNeededForDir(MAXDE), locOfNewDir);
            int writeReturn;
            if (writeReturn = LBAwrite(freeSpaceMap, 5, 1) != 5){
                printf("Error Writing with LBAwrite, exiting program\n");
                exit(-1);
            }
            return locOfNewDir;
        }
        i++;
    }
    printf("Cannot make new dir, parent dir is full\n");
    return -1;
}

//
fdDir *fs_opendir(const char *pathname)
{
    printf("Entering opendir\n");
    // 1. parse the pathname, make sure path is valid and find the last element
    struct fdPathResult tempPath = parsedPath(pathname);
    printf("tempPath***pathname: %s\n", pathname);
    printf("tempPath.dirPtr: %d\n", tempPath.dirPtr);
    printf("tempPath.index: %d\n", tempPath.index);

    // 2. check the last element to see if it is a directory
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), tempPath.dirPtr);
    // printf("tempBuffer[tempPath.index].fileType: %d\n", tempBuffer[tempPath.index].fileType);

    //      a: yes if last Arg type  IS directory
    //      b: no -> fail return null it is not a directory
    if (tempBuffer[tempPath.index].fileType != FT_DIRECTORY)
    {
        printf("return NULL from opendir\n");
        return NULL;
    }

    // 3.  Load this directory
    
    // load directory into starting location for LBAread
    fdDir* fd = malloc(sizeof(fdDir));
    fd->directoryStartLocation = tempBuffer[tempPath.index].location;
    

    printf("fd->directoryStartLocation: %ld\n", fd->directoryStartLocation);

    
    fd->dirEntryPosition = 0;
    fd->dirp_fs = malloc(sizeof(fs_diriteminfo));
    printf("return from opendir\n");
    return fd;
}

// takes a pointer and returns a pointer to fs_diriteminfo struct
// fs_diriteminfo contains d_name, filetType, d_reclen
// this is just do a for loop
// returns a pointer to fs_diriteminfo struct
// return null when Error occurs

fs_diriteminfo *fs_readdir(fdDir *fd){
   // start from where we last left off, which was position 0
    printf("Entering readdir\n");
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
            printf("Returning from readdir\n");
            return fd->dirp_fs;

        }
    }
    printf("Returning NULL from readdir\n");
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
