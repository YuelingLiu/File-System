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


#include "mfs.h"
#include "VCB.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


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
        char * tokenArray[50];
        int tokenIndex =0;

        char * token = strtok(path, "/");

        do{
            tokenArray[tokenIndex++] = token;
        } while (token = strtok(NULL, "/"));

        // im thinking we should load in root directory first
        struct VCB temp;
        printf("locOfRoot: %d\n", temp.numBlocks);


        struct fdPathResult result;
        result.dirPtr = 0;
        result.index = 0;
        
    }

    
}

int main(int argc, char const *argv[])
{
    char * temp = malloc (sizeof(char) * 40);
    strcpy(temp, "/hello/testing");
    parsedPath(temp);
    return 0;
}



