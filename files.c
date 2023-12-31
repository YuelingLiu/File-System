/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: files.c
*
* Description: This is a file for file allocation functions.
*
**************************************************************/

#include "files.h"



//Duplicate of asmt5 get file info
//This function should find file by calling parsepath
fileInfo * GetFileInfo (char * fname){
    
//Copy info from file's DE into fileInfo struct, return ptr to struct
    struct fdPathResult path = parsedPath(fname);
    if (path.index == -1)
    {
        return NULL;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);
    DirectoryEntry* tempDir = calloc (dirBlocks, vcb->blockSize);
    LBAread(tempDir, dirBlocks, path.dirPtr);

    fileInfo* fi = malloc(sizeof(fileInfo));
    
    if (tempDir[path.index].fileType == FT_DIRECTORY){
        printf("Cannot get file info for a directory!\n");
        return NULL;
    }
    
    //strncpy(fi->fileName, tempDir[path.index].name, 64);
    // we removed this and changed it to fname so that we save the full path.
    strncpy(fi->fileName, fname, 64);
  
    fi->fileSize = tempDir[path.index].size;

    fi->location = tempDir[path.index].location;
    free(tempDir);
    return fi;
}

//Writes a new index block to disk and returns its disk location
int createIndexBlock(){
    //printf("start createindexblock\n");
    LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    //Index block is exactly 512 bytes in size, enough to hold 64 location integers
    int* indexBlock = calloc(1, vcb->blockSize);

    int blockLocation = allocSingleBlock(freeSpaceMap, getFreespaceSize(vcb->numBlocks,
     vcb->blockSize));
    for (int i = 0; i < (vcb->blockSize/sizeof(int)); i++){
        indexBlock[i] = -1;
    }
    LBAwrite(indexBlock, 1, blockLocation);
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);

    free(indexBlock);
    return blockLocation;
}

//Given a path/filename of a new file, this function creates 
//a directory entry and index block for the new file
int makeNewFile(const char* pathname){
    //printf("start makenewfile\n");
    struct fdPathResult path = parsedPath(pathname);
    

    // if the file already exists we dont need to make another
    if (path.index != -1)
    {
        return -1;
    }
    //Load directory where new file is to be located
    DirectoryEntry* directory = calloc(blocksNeededForDir(50), vcb->blockSize);
    LBAread(directory, blocksNeededForDir(50), path.dirPtr);

    int i = 2; // starting dir index of NOT "." or ".."
    while (i < 50){
        if (strcmp(directory[i].name, "") == 0){ // Upon finding first available DE slot
            

            //Prepare index block of new file
            int locOfIndexBlock = createIndexBlock();

            // Prepare DE of new file
            strcpy(directory[i].name, path.lastArg);
            directory[i].size = 0;
            directory[i].fileType = FT_REGFILE;
            directory[i].numOfDE = 0;
            directory[i].location = locOfIndexBlock;
            // this location isnt updated 

            //Write directory containing new file back to disk
            LBAwrite(directory, blocksNeededForDir(50), path.dirPtr);
            
            free(directory);
            return locOfIndexBlock;
        }
        i++; // added this iterator to look for the next open slot
    }
    printf("Cannot make new file, directory is full\n");
    free(directory);
    return -1;
}

//Given the location of a file's index block and an index inside that block,
//this function allocates a new chunk and saves its location to that index
//also returns the location of the new chunk
int makeFileChunk(int indexBlockLoc, int index){
    LBAread(freeSpaceMap,5,vcb->locOfFreespace);
    int* indexBlock = calloc(1, vcb->blockSize);
    LBAread(indexBlock, 1, indexBlockLoc);

    int locOfNewChunk = allocSingleBlock(freeSpaceMap, getFreespaceSize(vcb->numBlocks,
     vcb->blockSize));

    
    indexBlock[index] = locOfNewChunk;
    LBAwrite(indexBlock, 1, indexBlockLoc);
    LBAwrite(freeSpaceMap,5,vcb->locOfFreespace);
    free(indexBlock);
    return locOfNewChunk;

}

//Similar to above function, but given an amount of bytes to be written to a file,
//this function will request a variable number of chunks and assign to given IB.
//If an IB is filled, a new IB will be created and chained to previous IB
int initializeWritableChunks(int indexBlockLoc, int count){
    //printf("start initializewrittablechunks\n");
    LBAread(freeSpaceMap,5,vcb->locOfFreespace);
    
    //Based on count, figure out how many total chunks need to be added
    int numChunks = 1;
    while ((numChunks*512) < count){
        //If not enough to cover the count, add twice as many chunks!
        numChunks += numChunks * 2;
    }
    
    //Load given index block
    int* indexBlock = calloc(1, vcb->blockSize);
    LBAread(indexBlock, 1, indexBlockLoc);

    //Prepare variables needed to track progress through index blocks
    int IBIndex = 0;
    int currentBlockLoc = indexBlockLoc;

    //Set tracker to first unused index
    while (indexBlock[IBIndex] != (-1) && IBIndex < 63){
        IBIndex++;
    }

    //Until no more chunks left to be written, alloc free block/chunk and assign to index block
    while (numChunks > 0){
        //If tracker is at last index of Index Block, then we need to create a new Index Block
        int lastIndex = (vcb->blockSize/sizeof(int)) - 1;
        if (IBIndex == lastIndex){
            indexBlock[IBIndex] = createIndexBlock();
            LBAwrite(indexBlock, 1, currentBlockLoc);
            
            //Transition to working with new block
            currentBlockLoc = indexBlock[IBIndex];
            LBAread(indexBlock, 1, currentBlockLoc);
            IBIndex = 0;
        }
        
        indexBlock[IBIndex] = allocSingleBlock(freeSpaceMap, getFreespaceSize(vcb->numBlocks,
         vcb->blockSize));
        IBIndex++;
        numChunks--;
    }
    LBAwrite(freeSpaceMap,5,vcb->locOfFreespace);

    LBAwrite(indexBlock, 1, currentBlockLoc);
    //Return location of most recent IB added
    free(indexBlock);
    return currentBlockLoc;

}

//Returns the location of the nth block of a file
//represented by fileInfo* fi
int getBlockN(int n, fileInfo* fi){
    
    int blockNumber = n / ((vcb->blockSize-sizeof(int))/sizeof(int));
    int indexInBlock = n % ((vcb->blockSize-sizeof(int))/sizeof(int));
    int* temp = calloc(1, vcb->blockSize);
    LBAread(temp, 1, fi->location);
    int i = 0;
    int next;
    int lastIndex = (vcb->blockSize/sizeof(int)) - 1;
    while (i < blockNumber){        
        next = temp[lastIndex];
        if (next == -1){
            printf("Index block for chunk n doesn't exist!\n");
            return 0;
        }
        LBAread(temp, 1, next);
    }
    int locN = temp[indexInBlock];
    free(temp);
    return locN;

}

//Given n-th chunk of a file (from 0), return the location of the index block
//that contains the pointer to that chunk.
int getIndexBlockLoc(int chunkNumber, fileInfo* fi){
    int blockNumber = chunkNumber / ((vcb->blockSize-sizeof(int))/sizeof(int));
    if (blockNumber == 0){
        return fi->location;
    }
    int* temp = calloc(1, vcb->blockSize);
    LBAread(temp, 1, fi->location);
    int i = 0;
    int next;
    int lastIndex = (vcb->blockSize/sizeof(int)) - 1;
    while (i < (blockNumber - 1)){
        next = temp[lastIndex];
        if (next == -1){
            printf("Index block for chunk n doesn't exist!\n");
            return 0;
        }
        LBAread(temp, 1, next);
    }
    int loc = temp[lastIndex];
    free(temp);
    return loc;
}



