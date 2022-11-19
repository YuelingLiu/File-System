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


int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].fi = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].fi == NULL)
			{
			fcbArray[i].fi = (fileInfo *)-2;
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
int b_open (char * filename, int flags)
	{
	printf("printing inside of b_open\n");
	int returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's

	//Set to read mode, write mode, or read/write mode
	if ((flags & O_RDONLY) == O_RDONLY){
		fcbArray[returnFd].mode = O_RDONLY;
	}
	else if ((flags & O_WRONLY) == O_WRONLY){
		fcbArray[returnFd].mode = O_WRONLY;
	}
	else if ((flags & O_RDWR) == O_RDWR){
		fcbArray[returnFd].mode = O_RDWR;
	}
	
	//Case O_CREAT: Must create file first!
	if ((flags & O_CREAT) == O_CREAT){
		makeNewFile(filename);

	}
	
	fcbArray[returnFd].fi = GetFileInfo(filename);
	fcbArray[returnFd].localBuff = calloc(1, B_CHUNK_SIZE);
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].chunkNumber = 0;
	fcbArray[returnFd].currentIndexBlockLoc = fcbArray[returnFd].fi->location;
	fcbArray[returnFd].buflen = 0;

	//Case O_APPEND: TODO
	if ((flags & O_APPEND) == O_CREAT){
		//set index, chunkNumber, etc. to end of file
	}
	
	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
	return (0); //Change this
	}



// Interface to write function	

/* 
*write() writes up to count bytes from the buffer starting at buf
to the file referred to by the file descriptor fd.

On success, the number of bytes written is returned.
*/
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if (fcbArray[fd].mode == O_RDONLY){
		return (-1); //Invalid mode: cannot write to readonly file
	}

	printf("*****************************************\n");
	printf("Start of b_write\n");
	
	//*variables
	int finalCount = count;			// numBytes to be processed

	//Decrement count every time we write 512 byte chunk 

	// while (count > 0){
	// 	int locN = getBlockN(fcbArray[fd].chunkNumber, fcbArray[fd].fi);
	// 	//Case 1: chunk is already allocated and/or partially filled
	// 	//Must fill this chunk before writing it back and moving on
	// 	if ( locN != -1) {
	// 		LBAread(fcbArray[fd].localBuff, 1, locN);
	// 		memcpy(fcbArray[fd].localBuff + fcbArray[fd].index, buffer, B_CHUNK_SIZE - fcbArray[fd].index);

	// 	}
	// }
	
	// *while loop for if the **USER COUNT > B_CHUNKSIZE**
	while(count > B_CHUNK_SIZE){
		// first grab the remainder of fileChunkOffset and write to it
		// else, if fileChunkOffset is 0, finalCount will decrement 512 chunks at a time
		finalCount -= (B_CHUNK_SIZE - fcbArray->index);
		
		// load the fileChunk from ArrayBlockLoc
		LBAread(fcbArray->localBuff, 1, fcbArray->fi->location);

		// copy from user buffer to process buffer
		memcpy(fcbArray->localBuff, buffer, (B_CHUNK_SIZE - fcbArray->index));

		// write localBuff to storage
		LBAwrite(fcbArray->localBuff, 1, fcbArray->fi->location);

		// assign FCO to 0 so the next loop iteration grabs 512 chunks
		fcbArray->index = 0;

		// iterate through the ArrayBlock and load in the next location
		fcbArray->chunkNumber++;
	}
	
	
	//If chunk is not yet allocated (-1 address in index block)
	// 
	
		//int indexInIndexBlock = fcbArray[fd].chunkNumber % (INDEXBLOCKSIZE/INTSIZE);
		//makeFileChunk(fcbArray[fd].currentIndexBlockLoc, indexInIndexBlock);

	
	// *base case in the case that the other previous if statements dont run
	LBAread(fcbArray->localBuff, 1, fcbArray->fi->location);
	fcbArray->index += count;

	memcpy(fcbArray->localBuff, buffer, count);
	LBAwrite(fcbArray->localBuff, 1, fcbArray->fi->location);






	return (0); //Change this
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
//When we LBA read, use getBlockN() function
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		fcbArray[fd].fi = NULL;
		free(fcbArray[fd].localBuff);

	}
