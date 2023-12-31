/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
* Project: Basic File System
*
* File: mfs.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
//#include "b_io.h"
#include "DE.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include "fsLow.h"
//#include "mfs.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"


#include <dirent.h>
#define MAXDE 50
#define MAXLENGTH 256
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif



// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
typedef struct 
	{
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;     // if file or directory
    char d_name[256]; 			/* filename max filename is 255 characters */
	} fs_diriteminfo;

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short  d_reclen;		/*length of this record */
	unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
	//DirectoryEntry*  dir; // I added this directory pointer
	char d_name[256];               
	fs_diriteminfo *dirp_fs; // This a nested struct pointer that points to fs_diriteminfo
	//fileInfo * GetFileInfo (char * fname);
	} fdDir;


extern char globalPath[MAXLENGTH];
extern DirectoryEntry *tempBuffer;
//extern fdDir* fd;
//extern fs_diriteminfo *retTempDir; 

struct fdPathResult{
	int dirPtr;					// pointer to the directory 
	int index;					// index of file/directory
	char lastArg[20];				// the name of the final file /foo/"bar"
};

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);
fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char * fs_getcwd( char *pathname, size_t size);
int fs_setcwd(char *pathname);  //linux chdir
int fs_isFile(char * filename);	//return 1 if file, 0 otherwise
int fs_isDir(char * pathname);	//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file
void fs_pathReconstruction (); // edge case for . and .. 

//FOR USE WITH FS_DELETE
void markChunksFree(int indexLoc);


// This is the structure that is filled in from a call to fs_stat
struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *buf);

struct fdPathResult parsedPath(const char * path);


#endif

