/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: VCB.h
*
* Description: This is the header file for the volume control block.
*
* Make sure to set the #defined on the CMDxxxx_ON from 0 to 1 
* when you are ready to test that feature
*
**************************************************************/

#ifndef VCB_H
#define VCB_H

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

//Ensuring the VCB is accessible from anywhere
extern VCB* vcb;

#endif


