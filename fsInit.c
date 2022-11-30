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
#include "VCB.h"
#include "DE.h"
#include "freespace.h"


#define MAXDE 50


//Extern global variables available to all files
VCB* vcb;

uint8_t *freeSpaceMap;

DirectoryEntry *tempBuffer;
//fdDir *fd;
//fs_diriteminfo *retTempDir;

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
	//DirectoryEntry directoryEntries[MAXDE];
    DirectoryEntry* directoryEntries = calloc(blocksNeeded, blockSize);

	// loop through and initialize each directory entry structure to be in a known free state
	for(int i = 0 ; i < MAXDE; i++){
        strcpy(directoryEntries[i].name, "");
        //printf("directoryEntries[i].name: %s\n", directoryEntries[i].name);
	}

	// 6. Ask the free space for 6 blocks, and it should return
	// a starting block number for those 6 blocks
    freeSpaceMap = malloc(FSSize);
    LBAread(freeSpaceMap, 5, 1);

    // Call allocContBlocks allocate contiguous blocks function
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


    // Call LBAwrite function to write directoryEntires ,the number of blocksNeeded 
    // and start from the location of the root 
    LBAwrite(directoryEntries, blocksNeeded, locOfRoot);

    LBAwrite(freeSpaceMap, 5, 1);

    free(directoryEntries);
    return locOfRoot;
}

int initFreespace(size_t fssize) {
    //freeSpaceMap = malloc(fssize);
    freeSpaceMap = calloc(5, 512);

    // set the first 6 bits to 1 for the VCB and the bitmap
    for (size_t i = 0; i <= 5; i++){
        setBitOne(freeSpaceMap, i);
    }

    // block 1 is where freespace will be written
    LBAwrite(freeSpaceMap, 5, 1);


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

    tempBuffer = malloc(sizeof(DirectoryEntry) * MAXDE);

    int initialRead;
	if (initialRead = LBAread(vcb, 1, 0) != 1){
		printf("Error reading with LBAread, exiting program\n");
		exit(-1);
	}

    // You now have a pointer to a structure, so look at the signature (magic number)
    // in your structure and see if it matches.

    // temp to get the code to work MAKE SURE TO DELETE
    //vcb->signature = 12;

    if (vcb->signature != MAGICNUM) {
        vcb->signature = MAGICNUM;
        vcb->numBlocks = numberOfBlocks;
        vcb->blockSize = blockSize;
        vcb->locOfFreespace = initFreespace(FSSize); // function to be implemented
        vcb->locOfRoot = initRootDE(blockSize, FSSize); // function to be implemented


		// after the values are populated into the VCB, write to storage.
        int writeReturn;
		if (writeReturn = LBAwrite(vcb, 1, 0) != 1){
            printf("Error Writing with LBAwrite, exiting program\n");
		    exit(-1);
        }
        return 0;
    }
    else {
		// volume has already been formatted so no changes to the vcb
        freeSpaceMap = calloc(5, 512);
        LBAread(freeSpaceMap, 5, 1);
        printf("*****freeSPaceMapCalloc %p\n",freeSpaceMap);
        return 0;
    }

}




void exitFileSystem () {
	printf ("System exiting\n");

    free(vcb);
    printf ("System exiting1\n");
    vcb = NULL;
    printf ("System exiting1.5\n");
    printf ("freespaceMap%p\n",freeSpaceMap);
    free(freeSpaceMap);
    printf ("System exiting2\n");
    freeSpaceMap = NULL;
}