/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: DE.c
*
* Description: This is the file for our directory entry.
*
**************************************************************/

#include "DE.h"

int blocksNeededForDir(int maxde) {
    int bytesNeeded = maxde * sizeof(DirectoryEntry);
    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1)) / vcb->blockSize;
    return blocksNeeded;
}
