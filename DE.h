/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: DE.h
*
* Description: This is the header file for our directory entry.
*
**************************************************************/

#ifndef DE_H
#define DE_H
#include "VCB.h"

typedef struct DirectoryEntry {
    // the name of the entry that is unique to that file, and is used for lookup
    char name[256];

    // the size of the file so we know how far to read up to
    long size;

    // Dictate the total number of directory entries we want for a directory.
    int numOfDE;

    // Dictate the number of bytes we need.
    // (multiply the size of directory entry by the number of entries)
    int bytesNeeded;

    // the location that we want to keep
    long location;

    // including a filetype for directories and files
    unsigned char fileType;

} DirectoryEntry;

int blocksNeededForDir(int maxde);

#endif
