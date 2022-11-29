/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "fsLow.h"
#include "mfs.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"
#include "files.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef int b_io_fd;

typedef struct b_fcb {
	/** TODO add al the information you need in the file control block **/
	fileInfo* fi;

	char * localBuff;		// holds the open file buffer
	int chunkOffset;		// holds the current position in the buffer
	int chunkNumber;		// n-th (from 0) 512 byte chunk of the file
	int currentIndexBlockLoc;
	int buflen;			// holds how many valid bytes are in the buffer
	int mode; 			// O_RDONLY, O_WRONLY, or O_RDWR
} b_fcb;

int b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif
