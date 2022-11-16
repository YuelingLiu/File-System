typedef struct fileInfo {
	char fileName[64];		//filename
	int fileSize;			//file size in bytes
	int location;			//starting lba (block number) for the file data
} fileInfo;

fileInfo * GetFileInfo (char * fname);

int createIndexBlock();  
	
int getBlockN(int n, DirectoryEntry* de);