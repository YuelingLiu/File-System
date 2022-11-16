#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			
#include <string.h>			
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "b_io.h"
#include "fsLow.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"
#include "files.h"
#include "mfs.h"

#define INDEXBLOCKSIZE 512
#define INTSIZE 8

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
    LBAwrite(indexBlock, 1, blockLocation);
    free(indexBlock);
    return blockLocation;
}

int getBlockN(int n, DirectoryEntry* de){
    if (de->fileType != FT_REGFILE){
        return -1;
    }
    
    int blockNumber = n / (INDEXBLOCKSIZE/INTSIZE);
    int indexInBlock = n % (INDEXBLOCKSIZE/INTSIZE);
    int* temp = calloc(1, INDEXBLOCKSIZE);
    LBAread(temp, 1, de->location);
    int i = 0;
    int next;
    while (i < blockNumber){
        next = temp[63];
        LBAread(temp, 1, next);
    }
    int locN = temp[indexInBlock];
    return locN;

}

