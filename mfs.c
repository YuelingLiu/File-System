/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: mfs.c
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/


// #include "mfs.h"
// #include "VCB.h"
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include "DE.h"
// #include "fsLow.h"
//#include "fsInit.c"
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "fsLow.h"
#include "mfs.h"
#include "VCB.h"
#include "DE.h"
#include "freespace.h"

#define MAXDE 50


int fs_rmdir(const char *pathname){
    // fd_PathResult = fd_ParsePath(pathname);
    struct fdPathResult path;
    path.dirPtr = 30;
    path.index = 5;

    // pointer to struct = fs_opendir(pathname)
    






    //fs_close
}


struct fdPathResult parsedPath(char * path){
    // check if absolute or relative
    char firstChar = path[0];
    int isAbsolute = 0;
    
    // this works
    if (strcmp(&firstChar, "/") == 0){
        isAbsolute = 1;
    }

    if (isAbsolute == 1){
        // make an array to load in data
        char * tokenArray[50];         // array of names to be tokenized
        const char s[2] = "/";         // delimiter
        int tokenIndex = 0;            // counter for number of tokens
        char str[strlen(path)];        // declare a string, str to be read of size strlen(path)
        strcpy(str,path);              // copy path into str


        // loop to tokenize values 
        char * token = strtok(str, s); 

        while (token != NULL){
            tokenArray[tokenIndex++] = token;
            //printf("token: %s\n", token);
            token = strtok(NULL, s);
        }

        // for (size_t i = 0; i < tokenIndex; i++)
        // {
        //     printf("tokenArray[i]: %s\n", tokenArray[i]);
        // }
        
        // do{
        //     //printf("token: %s\n", token);
        //     tokenArray[tokenIndex++] = token;
        // } while (token = strtok(NULL, "/"));



        //printf("tokenIndex: %d\n", tokenIndex);

        // load in root directory first
        // we know that its at location 6 

        // MAXDE requires include "fsinit.c" but multiple definitions
        DirectoryEntry *tempRoot = malloc(sizeof(DirectoryEntry) * MAXDE);
        
        // locOfRoot is initialized at location 6
        LBAread(tempRoot, blocksNeededForDir(MAXDE), vcb->locOfRoot);
        //printf("vcb1.signature: %ld\n", vcb1->signature);
        for (size_t i = 0; i < 10; i++)
        {
            // if(strcmp(tempRoot[i].name,"") == 0){
            //     printf("testing to make sure it works\n");
            // }
            printf("tempRoot[i].name: %s\n", tempRoot[i].name);
         }
        

        struct fdPathResult result;
        result.dirPtr = 0;
        result.index = 0;
        
    }

    
}

int fs_isFile(char * filename){

}	//return 1 if file, 0 otherwise



int fs_isDir(char * pathname);	//return 1 if directory, 0 otherwise





// Misc directory functions
// This function is to get the working directory for the current task
// Returns a pointer to pathname, and pointer so NULL otherwise 
char * fs_getcwd(char *pathname, size_t size){
    //copy the abosulte pathname for the current working directory

}
int fs_setcwd(char *pathname);  //linux chdir
