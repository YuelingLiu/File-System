/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "fsLow.h"
#include "mfs.h"
//#include "mfs.c"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"

#define MAXDE 50




//Extern global variable available to all files
VCB* vcb;

uint8_t *freeSpaceMap;

// static array of directory entries with a number 50
// DirectoryEntry directoryEntries[MAXDE];
// probably remove this later

long MAGICNUM = 133713371337;



// The root directory follows the bitmap blocks
int initRootDE(int blockSize, int FSSize){
	// 1. First, we need space.
	// 2. Initialize how many directory entries we want for a directory.
	// 3. Multiply the size of directory entry by the number directory entries.
    int bytesNeeded = MAXDE * sizeof(DirectoryEntry);

	// 4. Determine how many blocks we need. 19531 blocks, bits blockSize: 512
    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;

	// 5. Now we have a pointer to an array of directory entries.
	DirectoryEntry directoryEntries[MAXDE];

	// loop through and initialize each directory entry structure to be in a known free state
	for(int i = 0 ; i < MAXDE; i++){
        strcpy(directoryEntries[i].name, "");
        //printf("directoryEntries[i].name: %s\n", directoryEntries[i].name);
	}

	// 6. Ask the free space for 6 blocks, and it should return
	// a starting block number for those 6 blocks
    freeSpaceMap = malloc(FSSize);
    LBAread(freeSpaceMap, 5, 1);

    int locOfRoot = allocContBlocks(freeSpaceMap, FSSize, blocksNeeded);

    // set the dot
    strcpy(directoryEntries[0].name, ".");
    directoryEntries[0].size = MAXDE * sizeof(DirectoryEntry);
    directoryEntries[0].location = locOfRoot;
    directoryEntries[0].fileType = FT_DIRECTORY;
    directoryEntries[0].numOfDE = MAXDE;
    

    // set the dot dot
    strcpy(directoryEntries[1].name, "..");
    directoryEntries[1].size = MAXDE * sizeof(DirectoryEntry);
    directoryEntries[1].location = locOfRoot;
    directoryEntries[1].fileType = FT_DIRECTORY;
    directoryEntries[1].numOfDE = MAXDE;



    LBAwrite(directoryEntries, blocksNeeded, locOfRoot);



    LBAwrite(freeSpaceMap, 5, 1);


    return locOfRoot;
}

int initFreespace(size_t fssize) {
    uint8_t* freeSpaceMap = malloc(fssize);

    // set the first 6 bits to 1 for the VCB and the bitmap
    for (size_t i = 0; i <= 5; i++){
        setBitOne(freeSpaceMap, i);
    }

    // block 1 is where freespace will be written
    LBAwrite(freeSpaceMap, 5, 1);

    free(freeSpaceMap);
    freeSpaceMap = NULL;

    return 1; // returning location of freespace to VCB
}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize) {
	printf ("Initializing File System with %ld blocks with a block size of %ld\n",
     numberOfBlocks, blockSize);


	/* TODO: Add any code you need to initialize your file system. */
    size_t FSSize = getFreespaceSize(numberOfBlocks, blockSize);

    // the first block will be of size BLOCKSIZE
    // even if there is a lot of empty, that's how we do it

    // malloc a block of memory as your VCB pointer and LBAread block 0
	vcb = malloc(blockSize);
	// buffer and number of block, reading from the beginning

    int initialRead;
	if (initialRead = LBAread(vcb, 1, 0) != 1){
		printf("Error reading with LBAread, exiting program\n");
		exit(-1);
	}

    // You now have a pointer to a structure, so look at the signature (magic number)
    // in your structure and see if it matches.

    // temp to get the code to work
    vcb->signature = 12;

    if (vcb->signature != MAGICNUM) {
        vcb->signature = MAGICNUM;
        vcb->numBlocks = numberOfBlocks;
        vcb->blockSize = blockSize;
        vcb->locOfFreespace = initFreespace(FSSize); // function to be implemented
        vcb->locOfRoot = initRootDE(blockSize, FSSize); // function to be implemented

        /* TEST CODE */

        testPopulateStorage("/");
        
        struct fdPathResult tempPath = parsedPath("/banana2/apple2/pear2");
        

        printf("result.index: %d\n", tempPath.index);
        printf("result.dirPtr: %d\n", tempPath.dirPtr);
        printf("result.lastArg: %s\n", tempPath.lastArg);

        // test for isDir
        int retisDir = fs_isDir("/banana2/apple2");
        if (retisDir == 1){
            printf("the folder exists\n");
        }

        // test for getcwd
        int size = strlen("/banana2");
        printf("before getcwd\n");
        char * retPath = fs_getcwd("/banana2", size);
        printf("after getcwd\n");

        // test for isfile
        int retisFile = fs_isFile("/banana2/apple2/pear2");
        if (retisFile == 1){
            printf("the file exists\n");
        }

        
        // test for openDir
        //fs_opendir("/banana2/apple25");

        printf("SANITY CHECK after opendir\n");
        mode_t temp;
        fs_mkdir("/banana2/grape", temp);
        

        /* TEST CODE */
        
		// after the values are populated into the VCB, write to storage.
        int writeReturn;
		if (writeReturn = LBAwrite(vcb, 1, 0) != 1){
            printf("Error Writing with LBAwrite, exiting program\n");
		    exit(-1);
        }
        return 1;
    }
    else {
		// volume has already been formatted so no changes to the vcb
        return 0;
    }

}

void exitFileSystem () {
	printf ("System exiting\n");
    free(vcb);
    vcb = NULL;
    free(freeSpaceMap);
    freeSpaceMap = NULL;
}