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
DirectoryEntry *tempBuffer;
fdDir *fd;
char globalPath[MAXLENGTH];







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
//      }
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
void testPopulateStorage ( const char * path){
    
    //gotta figure this out
    printf("before\n");
    strcpy(globalPath, "/banana2/apple2");
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
        fd = malloc(sizeof(fdDir));
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
        LBAread (tempBuffer,12,0);
        // testing 

        // success. I am able to create directories inside the root
        // root = . .. banana banana2
        LBAread(tempBuffer,blocksNeededForDir(numberofDE), location);
        //printf("root: %s\n", tempBuffer[3].name);
        //printf("tempBuffer[3].numOfDE: %d\n", tempBuffer[3].numOfDE);
        
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
        LBAread (tempBuffer,12,0);
        //printf("tempBuffer[0].name: %s\n", tempBuffer[0].name);

        // success it prints out . and ..
        LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
        //printf("tempBuffer[0].name: %s\n", tempBuffer[2].name);

        // -> root -> level 1 need to go down one more layer
        // going into apple 2
        location = directoryEntries[3].location;
        numberofDE = directoryEntries[3].numOfDE;
        // apple 2 is at directoryEntries[3]
        LBAread(tempBuffer, blocksNeededForDir(numberofDE), location );

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

        LBAread (tempBuffer,12,0);
        //printf("tlevel 2: %s\n", tempBuffer[0].name);
        
        LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
        //printf("tempBuffer[0].name: %s\n", tempBuffer[3].name);


        /* TEST CODE */
    


   
}



struct fdPathResult parsedPath(const char * path){
    
    // check if absolute or relative
    char firstChar = path[0];
    int isAbsolute = 0;
    struct fdPathResult result;
    // confirm if the path is relative to absolute
    if (strcmp(&firstChar, "/") == 0)
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
    if(isAbsolute == 0){
        // grab in global current directory variable which is the absolute location
        char * currentDir = fs_getcwd(path,strlen(path));
        

        // tokenize the path passed in
        // tokenizes path into tokens
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
        
        // print tokens
        for (size_t i = 0; i < tokenIndex; i++)
        {
            printf("tokenArray[i]: %s\n", tokenArray[i]);
        }


        while (token = strtok(NULL, "/")){
            tokenArray[tokenIndex++] = token;
        }
        // end of tokenizer


        // from root given(root location)
        // parse the absolute location
        // starting from root, we go to next location, then next location 
        // until we get to end then finally we search for path.name

        
        strcat(currentDir, "/");

        strcat(currentDir, path);
        
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
        
        // print tokens
        for (size_t i = 0; i < tokenIndex; i++)
        {
            printf("tokenArray[i]: %s\n", tokenArray[i]);
        }


        while (token = strtok(NULL, "/")){
            tokenArray[tokenIndex++] = token;
        }
        // end of tokenizer


        // im using tempBuffer instead of tempRoot
        // create a variable that changes for the loop to run
        // commented out bc location is made in the test above
        int location = vcb->locOfRoot;

        // assign the last value in tokenArray to result last arg
        // save last arg
        strcpy(globalTemp.lastArg, tokenArray[tokenIndex-1]);

        int numberofDE = MAXDE;

        // loop through all of the tokens in root
        for (size_t i = 0; i < tokenIndex; i++)
        {
            LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
            int j = 0;

            // loop through the directory entries for name comparison
            while (j < numberofDE)
            {
                if (strcmp(tempBuffer[j].name, tokenArray[i]) == 0)
                {
                    // works up to here 100%
                   
                    //printf("tempBuffer[j].name: %s\n", tempBuffer[j].name);
                    
                    // this will update and it's to grab the final index
                    // index locaiton
                    location = tempBuffer[j].location;
                    //printf("location: %d\n", location);
                    numberofDE = tempBuffer[j].numOfDE;
                    //printf("numberofDE: %d\n", numberofDE);
                    //printf("tempBuffer[j].name22: %s\n", tempBuffer[j].name);

                    // once the comparison is found, we read in the next location
                    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
                    break;
                    
                }
                j++;
                // grabbing result
                if (i == tokenIndex -1 ){
                    globalTemp.index = j;
                    //printf("result.index: %d\n", result.index);
                    //break;
                    }

                // in the case that we loop through the entire directory entries
                if (j == numberofDE -1)
                {
                    printf("no directory with the name: %s\n", tokenArray[i]);
                    globalTemp.dirPtr = -1;
                    globalTemp.index = -1;
                    break;
                }
            }   
            // prints out 1 bc the first parameter is /./notbanana
            //printf("j: %d\n", j);


            // find pointer to directory n-1
            // this will update multiple times but that's intentional
            // this works
            if (i == tokenIndex - 2){
                globalTemp.dirPtr = tempBuffer[i].location;
            }

        }
       
    result.dirPtr = globalTemp.dirPtr;
    result.index = globalTemp.index;
    strcpy( result.lastArg, globalTemp.lastArg);

    
    return result;
     
     
    }
    free(tempBuffer);
    tempBuffer = NULL;
    
    //  printf("globalTemp.index: %d\n", globalTemp.index);
    //     printf("globalTemp.dirPtr: %d\n", globalTemp.dirPtr);
    //     printf("globalTemp.lastArg: %s\n", globalTemp.lastArg);

    // result.dirPtr = globalTemp.dirPtr;
    // result.index = globalTemp.index;
    // strcpy( result.lastArg,globalTemp.lastArg);


    // printf("result.index: %d\n", result.index);
    // printf("result.dirPtr: %d\n", result.dirPtr);
    // printf("result.lastArg: %s\n", result.lastArg);
    
    // return result;

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
        if (strcmp(tempBuffer[i].name, tempPath.lastArg) == 0){
            if(tempBuffer[i].fileType == FT_REGFILE){
                return 1;
            }
        }
    }
    
    return 0;
    

}


int fs_isDir(char *pathname) {
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
        if(strcmp(tempBuffer[i].name, tempPath.lastArg) == 0){
            printf("tempBuffer[i].name: %s\n", tempBuffer[i].name);
            if(tempBuffer[i].fileType == FT_DIRECTORY){
                return 1;
            }
            
        }
        // in the case that i is MAXDE and tempBuffer[i].name does not exist
        // return 0
        if (i == MAXDE -1 ){
            printf("file does not exist***\n");
            return 0;
        }
        
    }
    printf("file does not exist\n");
    return 0;
 }


// Misc directory functions
// This function is to get the working directory for the current task
// Returns a pointer to absolute pathname, and pointer so NULL otherwise


//getcwd version 2
char *fs_getcwd(const char *pathname, size_t size){
    // why does it need these parameters?
    if (strlen(globalPath) > size){
        return NULL;
    }
    return globalPath;
}


// Linux chdir 

int fs_setcwd(char *pathname){
    // check if the pathname starts in the root direcotry 
    if(pathname[0]!='/'){
        return -1;
    }
    strcpy(globalPath, pathname);
    return 0;

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
//int fs_mkdir(const char *pathname, mode_t mode)
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

//}

struct fs_diriteminfo * loadDir (DirectoryEntry temp){
    
}

// 
fdDir * fs_opendir(const char *pathname){
    printf("works inside of opendir\n");
 // 1. parse the pathname, make sure path is valid and find the last element 
        struct fdPathResult tempPath = parsedPath(pathname);
        
 // 2. check the last element to see if it is a directory  
        LBAread(tempBuffer, MAXDE, tempPath.dirPtr);
        // printf("tempBuffer[tempPath.index].fileType: %d\n", tempBuffer[tempPath.index].fileType);
    
 //      a: yes if last Arg type  IS directory
 //      b: no -> fail return null it is not a directory
        if (tempBuffer[tempPath.index].fileType != FT_DIRECTORY){
            printf("return NULL\n");
            return NULL;
        }

 // 3.  Load this directory 
//      dirp = loadDir(DE); this directory entry we know from step 2 

        // we need to create a temporary struct of type fs_diriteminfo
        // populate that temp struct with the values listed below



        // copy over the name
        // strcpy(fd->dirp_fs.d_name, tempBuffer[tempPath.index].name);
        // printf("fd->dirp_fs.d_name: %s\n", fd->dirp_fs.d_name);

        // // copy over the fileType
        // fd->dirp_fs.fileType = tempBuffer[tempPath.index].fileType;

        // struct fs_diriteminfo temp;
        // fd->dirp_fs.d_reclen = sizeof(temp);


//  4. set fd position to 0 
//    fd->dirEntryPosition=0;
//    fd->dirp=dirp; 
        // fd->dirEntryPosition = 0;
        // fd->dirp_fs = 
//    
//  5. return a pointer to fdDir struct 

 //  return fd; 

}


 // takes a pointer and returns a pointer to fs_diriteminfo struct 
 // fs_diriteminfo contains d_name, filetType, d_reclen
 // this is just do a for loop
 // returns a pointer to fs_diriteminfo struct 
 // return null when Error occurs 



//struct fs_diriteminfo *fs_readdir(fdDir *dirp){
   // start from where we last left off, which was position 0 

    
    // for(int i = dirp->dirEntryPosition ; i < total_direcotory_entries; i++){
        // if this directory is used, 
    //     if DirectoryEntryUsed(dirp->dirp[i]){
    //         // ii fs_diriteminfo 
    //         // copy the name from our directory entry to the struct 
    //         strcpy(fd->dirp->fs_diriteminfo.d_name, fd->dirp[i].name);
    //         fd-dirp->fs_diriteminfo.fileType= typedef(fd->dirp[i]);
    //         fd->position =i+1;

    //         return (fd->dirp->fs_diriteminfo);
    //     }
    // // }
    // return NULL

//}

// my version
// struct fs_diriteminfo *fs_readdir(fdDir *dirp){
//     //base case
//     if(dirp ==NULL){
//         return NULL;
//     }

//     for(int i = 0 ; i < MAXDE;i++){
//         // DirectoryEntryUsed by checking the d_reclen length or dirEntryPosition
//         if(dirp[i].d_reclen > 4){
//              // copy the name from our directory entry to the struct 
//             strcpy(dirp->dirp_fs.d_name,dirp[i].d_name);
//             dirp->dirp_fs.fileType = FT_DIRECTORY;
//             dirp->dirEntryPosition =i+1;

//           return &dirp->dirp_fs;
//     }

//     return NULL;

// }


// free up memory here  we allocated in the open
// int fs_closedir(fdDir *dirp){
  
// }

