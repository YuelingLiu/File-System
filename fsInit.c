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

long MAGICNUM = 133713371337;


void setBitOne(uint8_t **freeSpaceMap, int i){
    *freeSpaceMap[i >> 3] |= (1 << (i & 0x7));    
}

void setBitZero(uint8_t **freeSpaceMap, int i){
    *freeSpaceMap[i >> 3] &= (0 << (i & 0x7));    
}

bool getBit(uint8_t **freeSpaceMap, int i){
    return *freeSpaceMap[i >> 3] & (1 << (i & 0x7));
}

int initFreespace(int numberOfBlocks, int blockSize) {
    int bytesNeeded = (numberOfBlocks + 7) / 8;
    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
    uint8_t* freeSpaceMap = malloc(blocksNeeded * blockSize);
    

    // set the first 6 bits to 1 for the VCB and the bitmap
    for (size_t i = 0; i <= 7; i++)
    {
        setBitOne(&freeSpaceMap, i);
    }

    
    // for (size_t i = 0; i <=7   ; i++)
    // {
    //    printf("getBit: %d\n", getBit(&freeSpaceMap, i));
    // }

    printf("sizeof(freeSpaceMap): %d\n", strlen(freeSpaceMap));
    //ToDo: Set remaining bits as 0 (free)
    
    for (size_t i = 0; i <= 5; i++)
    {
        setBitZero(&freeSpaceMap, i);
    }
    


    // for (size_t i = 0; i <=5 ; i++)
    // {
    //    printf("getBit: %d\n", getBit(&freeSpaceMap, i));
    // }
    
    printf("getBit(&freeSpaceMap, ): %d\n", getBit(&freeSpaceMap, 7));

    //printf("freeSpaceMap[1]: %d\n", freeSpaceMap[1]);

    //Block 1 is where freespace will be written
    LBAwrite(freeSpaceMap, 5, 1); 
    return 1; //Returning location of freespace to VCB
}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

    // the first block will be of size BLOCKSIZE even if there is a lot of empty space that's how it do 
    // Malloc a Block of memory as your VCB pointer and LBAread block 0
	VCB* vcb = malloc(blockSize);
	
	//buffer and number of block, reading from the beginning 

	//                           TODO check and figure out of this checker is fine.Change variable name of larry
    int larry;
	if(larry = LBAread(vcb, 1, 0) != 1){
		printf("larry: %d\n", larry);
		
		printf("Error reading with LBAread, exiting program\n");
		exit(-1);
	}
	

    //You now have a pointer to a structure, so look at the signature (magic number) 
    //in your structure and see if it matches.
    printf("before comparison\n");
    printf("vcb->signature: %ld\n", vcb->signature);
    // test initialization . please delete
    vcb->signature = 12;
    if (vcb->signature != MAGICNUM) {
        printf("testing\n");
        vcb->signature = MAGICNUM;
        vcb->numBlocks = numberOfBlocks;
        vcb->blockSize = blockSize;
        printf("before free space init\n");
        vcb->locOfFreespace = initFreespace(numberOfBlocks, blockSize); //Function to be implemented
        //vcb->locOfRoot = initRoot(); //Function to be implemented
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
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}