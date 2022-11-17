#define INDEXBLOCKSIZE 512
#define INTSIZE 8

typedef struct fileInfo {
	char fileName[64];		//filename
	int fileSize;			//file size in bytes
	int location;			//starting lba (block number) for the file data
} fileInfo;

fileInfo * GetFileInfo (char * fname);

int createIndexBlock();  
int makeNewFile(const char* pathname);
int makeFileChunk(int indexLoc, int index);
	
int getBlockN(int n, fileInfo* fi);

