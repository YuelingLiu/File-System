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
    uint8_t* freeSpaceMap = malloc(FSSize);
    LBAread(freeSpaceMap, 5, 1);

    int locOfRoot = allocContBlocks(freeSpaceMap, FSSize, blocksNeeded);

    // set the dot
    strcpy(directoryEntries[0].name, ".");
    directoryEntries[0].size = MAXDE * sizeof(DirectoryEntry);
    directoryEntries[0].location = locOfRoot;
    directoryEntries[0].fileType = FT_DIRECTORY;
    

    // set the dot dot
    strcpy(directoryEntries[1].name, "..");
    directoryEntries[1].size = MAXDE * sizeof(DirectoryEntry);
    directoryEntries[1].location = locOfRoot;
    directoryEntries[1].fileType = FT_DIRECTORY;
    


    LBAwrite(directoryEntries, blocksNeeded, locOfRoot);



    LBAwrite(freeSpaceMap, 5, 1);

    free(freeSpaceMap);
    freeSpaceMap = NULL;

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

        parsedPath("/banana/apple");
        //mode_t test;
        //fs_mkdir("path", test);



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
}