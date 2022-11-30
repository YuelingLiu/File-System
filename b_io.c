/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include "b_io.h"

b_fcb fcbArray[MAXFCBS];

int startup = 0;	// indicates that this has not been initialized

// Method to initialize our file system
void b_init () {
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++) {
		fcbArray[i].fi = NULL; //indicates a free fcbArray
	}

	startup = 1;
}

// Method to get a free FCB element
b_io_fd b_getFCB () {
	for (int i = 0; i < MAXFCBS; i++) {
		if (fcbArray[i].fi == NULL) {
			fcbArray[i].fi = (fileInfo *)-2;
			return i;	// Not thread safe (But do not worry about it for this assignment)
		}
	}

	return (-1);  //all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
int b_open (char * filename, int flags) {
	printf("printing inside of b_open\n");
	int returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0) b_init();  //Initialize our system

	returnFd = b_getFCB();	// get our own file descriptor
							// check for error - all used FCB's

	// Set to read mode, write mode, or read/write mode

	if ((flags & O_RDONLY) == O_RDONLY) {
		printf("setting read only\n");
		fcbArray[returnFd].mode = O_RDONLY;
	}
	else if ((flags & O_WRONLY) == O_WRONLY) {
		printf("setting write only\n");
		fcbArray[returnFd].mode = O_WRONLY;
	}
	else if ((flags & O_RDWR) == O_RDWR) {
		printf("setting read write\n");
		fcbArray[returnFd].mode = O_RDWR;
	}

	// Case O_CREAT: Must create file first!
	if ((flags & O_CREAT) == O_CREAT) {
		makeNewFile(filename);
	}

	fcbArray[returnFd].fi = GetFileInfo(filename);
	if (fcbArray[returnFd].fi == NULL){
		return (-1);
	}
	fcbArray[returnFd].localBuff = calloc(1, B_CHUNK_SIZE);
	fcbArray[returnFd].chunkOffset = 0;
	fcbArray[returnFd].chunkNumber = 0;
	fcbArray[returnFd].currentIndexBlockLoc = fcbArray[returnFd].fi->location;
	fcbArray[returnFd].buflen = 0;

	// Case O_APPEND: TODO
	if ((flags & O_APPEND) == O_CREAT) {
		//set index, chunkNumber, etc. to end of file
	}

	return (returnFd); // all set
}


// Interface to seek function
int b_seek (b_io_fd fd, off_t offset, int whence) {
	if (startup == 0) b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); // invalid file descriptor
	}

	int currentOffset = (fcbArray[fd].chunkNumber * 512) + fcbArray[fd].chunkOffset;
	int newOffset; // adjust the new offset

	printf("*********************************************\n");
	printf("inside of seek function\n");

	// handle SEEK_SET
	if (whence == SEEK_SET) {
		// offset will be set at location offset from the beginning of the file
		int newOffset = offset;

		// adjust the new offset to our file allocation system.
		fcbArray[fd].chunkNumber = newOffset / B_CHUNK_SIZE;
		fcbArray[fd].chunkOffset = newOffset % B_CHUNK_SIZE;
/*--------------------------------------------100-------------------------------------------------*/
		fcbArray[fd].currentIndexBlockLoc = getIndexBlockLoc(fcbArray[fd].chunkNumber, 
			fcbArray[fd].fi);
		return newOffset;
	}

	// handle SEEK_CUR
	if (whence == SEEK_CUR) {
		// offset will be set at location offset from the previous offset of the file
		newOffset = currentOffset + offset;

		// adjust the new offset to our file allocation system.
		fcbArray[fd].chunkNumber = newOffset / B_CHUNK_SIZE;
		fcbArray[fd].chunkOffset = newOffset % B_CHUNK_SIZE;
/*--------------------------------------------100-------------------------------------------------*/
		fcbArray[fd].currentIndexBlockLoc = getIndexBlockLoc(fcbArray[fd].chunkNumber, 
			fcbArray[fd].fi);

		return newOffset;
	}

	// handle SEEK_END
	if (whence == SEEK_END) {
		// offset will be set from where the end of file (size) PLUS passed in offset
		newOffset = fcbArray[fd].fi->fileSize + offset;

		// adjust the new offset to our file allocation system.
		fcbArray[fd].chunkNumber = newOffset / B_CHUNK_SIZE;
		fcbArray[fd].chunkOffset = newOffset % B_CHUNK_SIZE;
		fcbArray[fd].currentIndexBlockLoc = getIndexBlockLoc(fcbArray[fd].chunkNumber, 
			fcbArray[fd].fi);

		printf("fcbArray[fd].chunkNumber: %d\n", fcbArray[fd].chunkNumber);
		printf("fcbArray[fd].chunkOffset: %d\n", fcbArray[fd].chunkOffset);

		return newOffset;
	}

	return (2); // Change this
}

// Interface to write function

/*
*write() writes up to count bytes from the buffer starting at buf
to the file referred to by the file descriptor fd.

On success, the number of bytes written is returned.
*/
int b_write (b_io_fd fd, char * buffer, int count) {
	printf("*****************************************\n");
	printf("Start of b_write\n");
	if (startup == 0) b_init();  //Initialize our system

	printf("after first if\n");

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); // invalid file descriptor
	}

	printf("after second if\n");
	//fcbArray[fd].mode = O_WRONLY;
	// where is fcbArray[fd].mode changed? *************

	// if (fcbArray[fd].mode == O_RDONLY) {
	// 	return (-1); // Invalid mode: cannot write to readonly file
	// }

	printf("after third if\n");
	printf("count: %d\n", count);

	// *variables
	int returnCount = count;
	int tempCount = count;	// numBytes to be processed
	int writeCount = 0;
	printf("tempCount: %d\n", tempCount);

	// Check if starting file chunk exists, if so grab it
	// If not, allocate the chunks we'll need
	int fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);

	// getBlockN returns -1 if chunk doesn't exist and 0 if index block doesn't exist
	if (fileChunk == (-1) || fileChunk == 0) {
		printf("failed first existence check\n");
		initializeWritableChunks(fcbArray[fd].currentIndexBlockLoc, count);
		fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
	}
	printf("after getblockn\n");
	// If starting part-way through starting file chunk, we finish filling that chunk first
	if (count > (B_CHUNK_SIZE - fcbArray[fd].chunkOffset) && fcbArray[fd].chunkOffset > 0) {
		printf("partial case because chunkOffset is: %d\n", fcbArray[fd].chunkOffset);
		LBAread(fcbArray[fd].localBuff, 1, fileChunk);
		memcpy(fcbArray[fd].localBuff + fcbArray[fd].chunkOffset, buffer,
			(B_CHUNK_SIZE - fcbArray[fd].chunkOffset));
		printf("partial case writing to location: %d\n", fileChunk);
		LBAwrite(fcbArray[fd].localBuff, 1, fileChunk);
		printf("fcbArray->localBuff: %s\n", fcbArray[fd].localBuff);

		writeCount += (B_CHUNK_SIZE - fcbArray[fd].chunkOffset);
		tempCount -= (B_CHUNK_SIZE - fcbArray[fd].chunkOffset);
		fcbArray[fd].chunkOffset = 0;
		fcbArray[fd].chunkNumber += 1;

		// After iterating chunk number, check for existence again?
		fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);

		// getBlockN returns -1 if chunk doesn't exist and 0 if index block doesn't exist
		if (fileChunk == (-1) || fileChunk == 0){
			printf("failed second existence check\n");
			initializeWritableChunks(fcbArray[fd].currentIndexBlockLoc, count);
			fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
		}
	}

	// *while loop for if the **USER COUNT > B_CHUNKSIZE**
	while (tempCount > B_CHUNK_SIZE) {
		printf("In write while loop, tempCount %d\n", tempCount);
		// first grab the remainder of fileChunkOffset and write to it
		// else, if fileChunkOffset is 0, tempCount will decrement 512 chunks at a time
		tempCount -= B_CHUNK_SIZE;

		// load the fileChunk from ArrayBlockLoc
		LBAread(fcbArray[fd].localBuff, 1, fileChunk);

		// copy from user buffer to process buffer
		memcpy(fcbArray[fd].localBuff, buffer + writeCount, B_CHUNK_SIZE);

		// write localBuff to storage
		printf("loop case writing to location: %d\n", fileChunk);
		LBAwrite(fcbArray[fd].localBuff, 1, fileChunk);
		printf("fcbArray->localBuff: %s\n", fcbArray[fd].localBuff);

		// iterate through the ArrayBlock and load in the next location
		fcbArray[fd].chunkNumber++;

		// grab the next fileChunk
		fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
		printf("while loop, next fileChunk: %d\n", fileChunk);

		writeCount += B_CHUNK_SIZE;
	}

	// *base case in the case that the other previous if statements dont run
	LBAread(fcbArray[fd].localBuff, 1, fileChunk);
	fcbArray[fd].chunkOffset += tempCount;

	printf("after LBAread\n");
	printf("tempCount: %d\n", tempCount);
	printf("writecount: %d\n", writeCount);
	memcpy(fcbArray[fd].localBuff, buffer + writeCount, tempCount);
	printf("base case writing to location: %d\n", fileChunk);
	LBAwrite(fcbArray[fd].localBuff, 1, fileChunk);
	printf("fcbArray->localBuff: %s\n", fcbArray[fd].localBuff);

	writeCount += tempCount;

	// Calculate new file size
	if (((fcbArray[fd].chunkNumber * 512) + fcbArray[fd].chunkOffset) >
	fcbArray[fd].fi->fileSize) {
		fcbArray[fd].fi->fileSize = ((fcbArray[fd].chunkNumber * 512) +
		fcbArray[fd].chunkOffset);
	}

	printf("new file size is: %d\n", fcbArray[fd].fi->fileSize);
	printf("end of Write\n");
	return (returnCount);
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
// When we LBA read, use getBlockN() function
int b_read (b_io_fd fd, char * buffer, int count) {

	if (startup == 0) b_init(); // Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS)) {
		return (-1); // invalid file descriptor
	}

	// if (fcbArray[fd].mode == O_WRONLY) {
	// 	return (-1); // Invalid mode: cannot read from writeonly file
	// }
	
	printf("************************************\n");
	printf("STARTING THE READ FUNCTION\n");

	int currentOffset = (fcbArray[fd].chunkNumber * B_CHUNK_SIZE) + fcbArray[fd].chunkOffset;
	printf("currentOffset: %d\n", currentOffset);
	int bytesRemaining = fcbArray[fd].fi->fileSize - currentOffset;
	printf("bytesRemaining: %d\n", bytesRemaining);
	
	if (bytesRemaining < count){
		count = bytesRemaining;
	}
	
	if (count == 0){
		return 0;
	}
	
	//Count to be incremented upon successful transfer to caller buffer
	//Used for function return value
	int bytesTransferred = 0;
	int fileChunk;

	

	if (count > B_CHUNK_SIZE){
		
		if (fcbArray[fd].chunkOffset > 0){
/*--------------------------------------------100-------------------------------------------------*/
			memcpy(buffer, fcbArray[fd].localBuff + fcbArray[fd].chunkOffset,
				B_CHUNK_SIZE-fcbArray[fd].chunkOffset);
			bytesTransferred += (B_CHUNK_SIZE - fcbArray[fd].chunkOffset);
			
			fcbArray[fd].chunkNumber++;
			fcbArray[fd].chunkOffset = 0; 
		}
		
		if ((count - bytesTransferred) > B_CHUNK_SIZE){ //Check to make sure count still over 512
			int i;
			for (i = 0; i < (count/B_CHUNK_SIZE); i++)
			{
			
				fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
				LBAread(buffer + bytesTransferred, 1, fileChunk);
			
				bytesTransferred += B_CHUNK_SIZE;
				
				fcbArray[fd].chunkNumber++;
			}
		}
		
		
		count = count - bytesTransferred; //Count should now be less than a block
	}

	//Proceed from here assuming less than block of bytes to copy
	//Local buffer empty? Grab new block from LBA
	if (fcbArray[fd].chunkOffset == 0){
		
		fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
		LBAread(fcbArray[fd].localBuff, 1, fileChunk);
	}
	//Case 1: Count less than remaining space in local buffer
	//Next call will resume current block
	if (B_CHUNK_SIZE - fcbArray[fd].chunkOffset > count){
/*--------------------------------------------100-------------------------------------------------*/
		memcpy(buffer + bytesTransferred, fcbArray[fd].localBuff +
			fcbArray[fd].chunkOffset, count);
		bytesTransferred += count;
		fcbArray[fd].chunkOffset += count;
		bytesRemaining -= bytesTransferred;
		return bytesTransferred;
	}
	//Case 2: Count equals remaining space in local buffer
	//Must allow new block to read from LBA in next call
	else if (B_CHUNK_SIZE - fcbArray[fd].chunkOffset == count){
/*--------------------------------------------100-------------------------------------------------*/
		memcpy(buffer + bytesTransferred, fcbArray[fd].localBuff +
			fcbArray[fd].chunkOffset, count);
		bytesTransferred += count;
		fcbArray[fd].chunkOffset = 0; //Reset block position to trigger LBAread on next call
		fcbArray[fd].chunkNumber += 1;
		bytesRemaining -= bytesTransferred;
		return bytesTransferred;
	}
	//Case 3: Not enough space in local buffer for requested count
	//Must fill block, transfer, request new block from LBA, then transfer again
	else if (B_CHUNK_SIZE - fcbArray[fd].chunkOffset < count){
/*--------------------------------------------100-------------------------------------------------*/
		memcpy(buffer + bytesTransferred, fcbArray[fd].localBuff +
			fcbArray[fd].chunkOffset, B_CHUNK_SIZE-fcbArray[fd].chunkOffset);
		bytesTransferred += (B_CHUNK_SIZE - fcbArray[fd].chunkOffset);
		count -= (B_CHUNK_SIZE - fcbArray[fd].chunkOffset);
		fcbArray[fd].chunkOffset = 0; //Reset for upcoming new block
		fcbArray[fd].chunkNumber += 1;
		//Fetch new block
		fileChunk = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
		LBAread(fcbArray[fd].localBuff, 1, fileChunk);

/*--------------------------------------------100-------------------------------------------------*/
		memcpy(buffer + bytesTransferred, fcbArray[fd].localBuff +
			fcbArray[fd].chunkOffset, count); 				
		bytesTransferred += count;
		fcbArray[fd].chunkOffset += count;
		bytesRemaining -= bytesTransferred;
		return bytesTransferred;

	}
	
}

// Interface to close the file
int b_close (b_io_fd fd) {
	// we need to populate the final count 
	
    struct fdPathResult path = parsedPath(fcbArray[fd].fi->fileName);
    if (path.index == -1)
    {
        return NULL;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);
    DirectoryEntry* tempDir = calloc (dirBlocks, vcb->blockSize);
    LBAread(tempDir, dirBlocks, path.dirPtr);

    tempDir[path.index].size = fcbArray[fd].fi->fileSize;

	LBAwrite(tempDir, dirBlocks, path.dirPtr);

    free(tempDir);
	
	free(fcbArray[fd].fi);
	fcbArray[fd].fi = NULL;
	free(fcbArray[fd].localBuff);
}
