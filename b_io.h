/**************************************************************
* Class:  CSC-415-0#  Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: b_io.h
*
* Description: Interface of basic I/O functions
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

#define MAXFCBS 20
#define B_CHUNK_SIZE 512



typedef int b_io_fd;

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	fileInfo* fi;
	
	char * localBuff;			//holds the open file buffer
	int index;					//holds the current position in the buffer
	int chunkNumber; 			//n-th (from 0) 512 byte chunk of the file
	int currentIndexBlockLoc;
	int buflen;					//holds how many valid bytes are in the buffer
	int mode; 					//O_RDONLY, O_WRONLY, or O_RDWR
	} b_fcb;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

