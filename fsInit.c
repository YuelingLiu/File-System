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
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#include "fsLow.h"
#include "mfs.h"

#define MAXDE 51

typedef struct VCB {
    // Dictate the total number of blocks in the volume
    int numBlocks;

    // Dictate how many bytes correspond to a single block
    int blockSize;

    // "Pointer" to first block of freespace bitmap
    int locOfFreespace;

    // "Pointer" to the first block of root directory
    int locOfRoot;

    // unique magic number to identify if the volume belongs to us
    long signature;
} VCB;

typedef struct DirectoryEntry {
    // the name of the entry that is unique to that file, and is used for lookup
    char name[256];

    // the size of the file so we know how far to read up to
    long size;

    // Dictate the total number of directory entries we want for a directory.
    int numOfDE;

    // Dictate the bytes we need multiply the size of your directory entry
    // by the number of entries
    int bytesNeeded;

    // Dictate the current time
    time_t timeStamp;

    // SUBJECT TO CHANGE: for now, going forward using a file allocation method
    // that requires a “pointer” to the starting block of the file.
    long location;

} DirectoryEntry;

// static array of directory entries with a number 50
DirectoryEntry directoryEntries[MAXDE];

long MAGICNUM = 133713371337;

void setBitOne(uint8_t *freeSpaceMap, int i){
    freeSpaceMap[i >> 3] |= (1 << (i & 0x7));
}

void setBitZero(uint8_t *freeSpaceMap, int i){
    freeSpaceMap[i >> 3] &= (0 << (i & 0x7));
}

bool getBit(uint8_t *freeSpaceMap, int i){
    return freeSpaceMap[i >> 3] & (1 << (i & 0x7));
}

// passing the value directly so we don't have to have ugly code
int getFreespaceSize(int numberOfBlocks, int blockSize){
    int bytesNeeded = (numberOfBlocks + 7) / 8;
    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
    return (blocksNeeded * blockSize);
}

// allocate contiguous blocks of free space for directories/files
int allocContBlocks(uint8_t *freeSpaceMap, size_t fssize, int num){
    // count how many contiguous free blocks there are starting from the first free one
    int freeBlockCounter = 0;

    // amount of used blocks inside the byte before the first free one
    int firstBitOffset = 0;

    // read in one byte at a time (8 bits, 1 byte block)
    for (size_t byteIndex = 0; byteIndex < fssize; byteIndex++){
        // check to confirm that it's not all 1s
        // if all bytes are 1111111 then that means there is no space
        // if there are zeroes then we have to traverse to find the first zero
        if (freeSpaceMap[byteIndex] != 255){
            // traverse through the byte until we find the first zero
            while(getBit(freeSpaceMap, (byteIndex * 8) + firstBitOffset) == 1){
                firstBitOffset++;
            }

            // after we found the first zero, traverse until we reach amount requested
            // or encounter a 1
            while(getBit(freeSpaceMap, (byteIndex * 8) + firstBitOffset + freeBlockCounter) == 0){
                freeBlockCounter++;

                // once freeBlockCounter is equal to userInput, we have found the space
                // starting at (byteIndex * 8) + firstBitOffset;
                if (freeBlockCounter == num){
                    for (int i = firstBitOffset; i < firstBitOffset + freeBlockCounter; i++){
                        setBitOne(freeSpaceMap, i); // mark the bits as used
                    }
                    return (byteIndex * 8) + firstBitOffset;
                }
            }
        }
        freeBlockCounter = 0;
        firstBitOffset = 0;
    }
    return -1;
}

// The root directory follows the bitmap blocks
int initRootDE(int blockSize, int FSSize){
	// 1. First, we need space.
	// 2. Initialize how many directory entries we want for a directory.
	// 3. Multiply the size of directory entry by the number directory entries.
    int bytesNeeded = MAXDE * sizeof(DirectoryEntry);

	// 4. Determine how many blocks we need. 19531 blocks, bits blockSize: 512
    int blocksNeeded = bytesNeeded / blockSize;

	// 5. Now we have a pointer to an array of directory entries.
	DirectoryEntry *arrayPtr;
	DirectoryEntry directoryEntries[MAXDE];
	arrayPtr = directoryEntries; // pointer points to the whole array

	// loop through and initialize each directory entry structure to be in a known free state
	for(int i = 0 ; i < MAXDE; i++){
        strcpy(directoryEntries[i].name, "");
	}

	// 6. Ask the free space for 6 blocks, and it should return
	// a starting block number for those 6 blocks
    uint8_t* freeSpaceMap = malloc(FSSize);
    LBAread(freeSpaceMap, 5, 1);

    int locOfRoot = allocContBlocks(freeSpaceMap, FSSize, blocksNeeded);
    printf("locOfRoot: %d\n", locOfRoot);

    // set the dot
    strcpy(directoryEntries[0].name, ".");
    directoryEntries[0].size = MAXDE * sizeof(DirectoryEntry);
    directoryEntries[0].location = locOfRoot;

    // set the dot dot
    strcpy(directoryEntries[1].name, "..");
    directoryEntries[1].size = MAXDE * sizeof(DirectoryEntry);
    directoryEntries[1].location = locOfRoot;

    LBAwrite(directoryEntries, blocksNeeded, locOfRoot);

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
	VCB* vcb = malloc(blockSize);
	// buffer and number of block, reading from the beginning

    int initialRead;
	if (initialRead = LBAread(vcb, 1, 0) != 1){
		printf("Error reading with LBAread, exiting program\n");
		exit(-1);
	}

    // You now have a pointer to a structure, so look at the signature (magic number)
    // in your structure and see if it matches.
    printf("before comparison\n");
    printf("vcb->signature: %ld\n", vcb->signature);

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
        return 1;
    }
    else {
		// volume has already been formatted so no changes to the vcb
        return 0;
    }

    free(vcb);
    vcb = NULL;
}

void exitFileSystem () {
	printf ("System exiting\n");
}