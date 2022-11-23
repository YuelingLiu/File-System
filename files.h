

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			
#include <string.h>			
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include "b_io.h"
#include "fsLow.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"
#include "mfs.h"



#define INDEXBLOCKSIZE 512
#define INTSIZE 4

typedef struct fileInfo {
	char fileName[64];		//filename
	int fileSize;			//file size in bytes
	int location;			//starting lba (block number) for the file data
} fileInfo;

fileInfo * GetFileInfo (char * fname);

int createIndexBlock();  
int makeNewFile(const char* pathname);
int makeFileChunk(int indexLoc, int index);
int initializeWritableChunks(int indexBlockLoc, int count);
	
int getBlockN(int n, fileInfo* fi);

