
#include "files.h"



//Duplicate of asmt5 get file info
//This function should find file by calling parsepath
fileInfo * GetFileInfo (char * fname){
//Copy info from file's DE into fileInfo struct, return ptr to struct
}

//Writes a new index block to disk and returns its disk location
int createIndexBlock(){
    //Index block is exactly 512 bytes in size, enough to hold 64 location integers
    int* indexBlock = calloc(1, INDEXBLOCKSIZE);
    int blockLocation = allocSingleBlock(freeSpaceMap, getFreespaceSize(vcb->numBlocks, vcb->blockSize));
    for (int i = 0; i < 64; i++){
        indexBlock[i] = -1;
    }
    LBAwrite(indexBlock, 1, blockLocation);
    free(indexBlock);
    return blockLocation;
}

//Given a path/filename of a new file, this function creates 
//a directory entry and index block for the new file
int makeNewFile(const char* pathname){
    printf("inside makeNewFile before parsed path\n");
    struct fdPathResult path = parsedPath(pathname);
    printf("inside makeNewFile after parsed path\n");


    // if the file already exists we dont need to make another
    if (path.index != -1)
    {
        return -1;
    }
    //Load directory where new file is to be located
    DirectoryEntry* directory = calloc(blocksNeededForDir(50), vcb->blockSize);
    LBAread(directory, blocksNeededForDir(50), path.dirPtr);

    printf("inside makeNewfile after LBAread\n");
    int i = 2; // starting dir index of NOT "." or ".."
    while (i < 50){
        printf("inside makenewfile after while loop\n");
        if (strcmp(directory[i].name, "") == 0){ // Upon finding first available DE slot
            
            printf("inside makenewfile inside loop\n");

            //Prepare index block of new file
            int locOfIndexBlock = createIndexBlock();

            // Prepare DE of new file
            strcpy(directory[i].name, path.lastArg);
            directory[i].size = 0;
            directory[i].fileType = FT_REGFILE;
            directory[i].numOfDE = 0;
            directory[i].location = locOfIndexBlock;

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
    
    int* indexBlock = calloc(1, INDEXBLOCKSIZE);
    LBAread(indexBlock, 1, indexBlockLoc);

    int locOfNewChunk = allocSingleBlock(freeSpaceMap, getFreespaceSize(vcb->numBlocks, vcb->blockSize));
    
    indexBlock[index] = locOfNewChunk;
    LBAwrite(indexBlock, 1, indexBlockLoc);

    return locOfNewChunk;

}

//Similar to above function, but given an amount of bytes to be written to a file,
//this function will request a variable number of chunks and assign to given IB.
//If an IB is filled, a new IB will be created and chained to previous IB
int initializeWritableChunks(int indexBlockLoc, int count){
    //Based on count, figure out how many total chunks need to be added
    int numChunks = 1;
    while ((numChunks*512) < count){
        //If not enough to cover the count, add twice as many chunks!
        numChunks += numChunks * 2;
    }
    
    //Load given index block
    int* indexBlock = calloc(1, INDEXBLOCKSIZE);
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
        if (IBIndex == 63){
            indexBlock[IBIndex] = createIndexBlock();
            LBAwrite(indexBlock, 1, currentBlockLoc);
            
            //Transition to working with new block
            currentBlockLoc = indexBlock[IBIndex];
            LBAread(indexBlock, 1, currentBlockLoc);
            IBIndex = 0;
        }
        
        indexBlock[IBIndex] = allocSingleBlock(freeSpaceMap, getFreespaceSize(vcb->numBlocks, vcb->blockSize));
        IBIndex++;
        numChunks--;
    }
    LBAwrite(indexBlock, 1, currentBlockLoc);
    //Return location of most recent IB added
    return currentBlockLoc;

}

//Returns the location of the nth block of a file
//represented by fileInfo* fi
int getBlockN(int n, fileInfo* fi){
    
    int blockNumber = n / (INDEXBLOCKSIZE/INTSIZE);
    int indexInBlock = n % (INDEXBLOCKSIZE/INTSIZE);
    int* temp = calloc(1, INDEXBLOCKSIZE);
    LBAread(temp, 1, fi->location);
    int i = 0;
    int next;
    while (i < blockNumber){
        next = temp[63];
        if (next == -1){
            printf("Index block for chunk n doesn't exist!\n");
            return 0;
        }
        LBAread(temp, 1, next);
    }
    int locN = temp[indexInBlock];
    return locN;

}



