/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: freespace.h
*
* Description: This is the header file for managing free space.
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

void setBitOne(uint8_t *freeSpaceMap, int i);

void setBitZero(uint8_t *freeSpaceMap, int i);

bool getBit(uint8_t *freeSpaceMap, int i);

int getFreespaceSize(int numberOfBlocks, int blockSize);

// allocate contiguous blocks of free space for directories/files
int allocContBlocks(uint8_t *freeSpaceMap, size_t fssize, int num);

int allocSingleBlock(uint8_t *freeSpaceMap, size_t fssize);

extern uint8_t *freeSpaceMap;

