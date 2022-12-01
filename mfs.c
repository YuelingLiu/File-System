/**************************************************************
* Class: CSC-415-03 Fall 2022
* Names: Tommy Truong, Yueling Liu, Steve Betts, Nicholas Hamada
* Student IDs: 913660519, 922272361, 921898143, 918602131
* GitHub Name: kpcrocks
* Group Name: dev/null
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

struct fdPathResult globalTemp;

//Make this extern so that its available everywhere
char globalPath[MAXLENGTH] = "/";


int fs_rmdir(const char *pathname)
{
    struct fdPathResult path = parsedPath(pathname);
    if (path.index == -1)
    {
        return -1;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);

    // Gain access to the directory we want to remove by reading in its parent directory
    DirectoryEntry * parentDir = calloc (dirBlocks, vcb->blockSize);
    
    LBAread(parentDir, dirBlocks, path.dirPtr);
    
    // Read in the directory we want to remove
    DirectoryEntry * dirToRemove = calloc (dirBlocks, vcb->blockSize);
    LBAread(dirToRemove, dirBlocks, parentDir[path.index].location);
    
    // Loop through dirToRemove, checking that each DE except "." and ".." is known free state
    for (int i = 2; i < MAXDE; i++)
    {
        if (strcmp(dirToRemove[i].name, "") != 0)
        {
            printf("can not remove directory because it is not empty.\n");
            return -1;
        }
    }

    // Mark blocks as free
    LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    for (int i = parentDir[path.index].location; i < parentDir[path.index].location +
    dirBlocks; i++)
    {
        setBitZero(freeSpaceMap, i);
    }

    // Set dirToRemove's DE to known free state
    strcpy(parentDir[path.index].name, "");
    parentDir[path.index].size = 0;
    parentDir[path.index].location = 0;
    parentDir[path.index].fileType = 0;
    parentDir[path.index].numOfDE = 0;
    

    // Write freespace and parentDir back to disk, free malloc
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
    LBAwrite(parentDir, dirBlocks, path.dirPtr);

    free(parentDir);
    free(dirToRemove);

    return 0;
}

//FOR USE WITH FS_DELETE
//Given location of starting index block, traverse and mark freespace bits
//for each file chunk and index block as free.
void markChunksFree(int indexBlockLoc){
    //printf("start markchunksfree\n");
    LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
    
    setBitZero(freeSpaceMap, indexBlockLoc);

    
    int* temp = calloc(1, vcb->blockSize);
    LBAread(temp, 1, indexBlockLoc);
    int next;
    int lastIndex = (vcb->blockSize/sizeof(int)) - 1;
    //Loop until no more index blocks in chain to load
    while(next != (-1)){
        //Interate through indexes in index block
        for (int i = 0; i < (vcb->blockSize/sizeof(int)); i++)
        {
            //If chunk is allocated, free it
            if (temp[i] != -1){
                setBitZero(freeSpaceMap, temp[i]);
            }
            
        }
        next = temp[lastIndex];
        if (next != -1){
            LBAread(temp, 1, next);
        }

    }
    free(temp);
    LBAwrite(freeSpaceMap, 5, vcb->locOfFreespace);
}

int fs_delete(char *filename)
     {
    struct fdPathResult path = parsedPath(filename);
    if (path.dirPtr == -1 && path.index == -1)
    {
        return -1;
    }
    int dirBlocks = blocksNeededForDir(MAXDE);

    // Gain access to the file we want to remove by reading in its parent directory
    DirectoryEntry * parentDir = calloc (dirBlocks, vcb->blockSize);
    LBAread(parentDir, dirBlocks, path.dirPtr);

    //OLD VERSION
    // // Get number of blocks being used by file
    // int fileBlocks = (parentDir[path.index].size + (vcb->blockSize - 1)) / vcb->blockSize;

    // // Mark blocks as free
    // for (int i = parentDir[path.index].location; i < parentDir[path.index].location +
    // fileBlocks; i++)
    // {
    //     setBitZero(freeSpaceMap, i);
    // }
    //NEW VERSION FOR INDEXED ALLOCATION
    markChunksFree(parentDir[path.index].location);

    // Set file's DE to known free state
    strcpy(parentDir[path.index].name, "");

    // Write freespace and parentDir back to disk

    // do we have to change the fileType to FT_REGFILE?

    //Write freespace and parentDir back to disk

    
    LBAwrite(parentDir, dirBlocks, path.dirPtr);

    free(parentDir);
    return 0;
}

//*parse
struct fdPathResult parsedPath(const char *path)
{
    // check if absolute or relative
    char firstChar = path[0];
    int isAbsolute = 0;
    struct fdPathResult result;
    // confirm if the path is relative to absolute
    if (firstChar == '/')
    {
        isAbsolute = 1;
        // in the case that the path is just the root
        
    }

    // for a relative path, we need to grab the path first
    /*
    relative path means you want to find the file/folder in the current directory
    first we need to get current directory (absolute path)
    then we need to tokenize the relative path. to grab the last argument
    we then concatenate the current directory with the absolute path
    once we have the entire absolute path, we run parsepath and this time,
    absolute value is 1 so it parses the entire path and returns fdPathResult
    */
    if (isAbsolute == 0)
    {
        // grab in global current directory variable which is the absolute location
        // char * currentDir = fs_getcwd(path,strlen(path));
        char currentDir[4096];
        strncpy(currentDir, globalPath, 4096);

        // from root given(root location)
        // parse the absolute location
        // starting from root, we go to next location, then next location
        // until we get to end then finally we search for path.name

        // this is an edge case such that if the currentDir is "/"
        // we dont want to add another "/" as that would set the
        // currentDir to "//"
        // so if the currdir is "/", we skip concatenating "/"
        // if the currDir is "/banana", we concatenate "/"
        if (strcmp(currentDir, "/") != 0 && strlen(currentDir) > 2)
        {
            strcat(currentDir, "/");
        }
        strcat(currentDir, path);

        struct fdPathResult tempPath = parsedPath(currentDir);

        result.dirPtr = tempPath.dirPtr;
        result.index = tempPath.index;
        strcpy(result.lastArg, tempPath.lastArg);
        return result;
    }

    // for absolute path, we will parse the path starting from root
    if (isAbsolute == 1)
    {
        // tokenizes path into tokens
        char *tokenArray[50];     // array of names to be tokenized
        
        const char s[2] = "/";    // delimiter
        int tokenIndex = 0;       // counter for number of tokens
        char str[strlen(path)];   // declare a string, str to be read of size strlen(path)
        strcpy(str, path);        // copy path into str
        
        // loop to tokenize values
        char *token = strtok(str, s);

        // tokenizes the values and inserts into tokenArray
        while (token != NULL)
        {
            tokenArray[tokenIndex++] = token;
            token = strtok(NULL, s);
        }

        
        // create a variable that changes for the loop to run
        // commented out bc location is made in the test above
        int location = vcb->locOfRoot;

        // assign the last value in tokenArray to result last arg
        // save last arg

        if (tokenIndex > 0){
            strcpy(globalTemp.lastArg, tokenArray[tokenIndex - 1]);
        }

        // edge case of just root inputted
        int numberofDE = MAXDE;
        
        // *loop
        // loop through all of the tokens in root
        for (int i = 0; i < tokenIndex; i++)
        {
            LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
            
            int j = 0;

            // loop through the directory entries for name comparison
            while (j < numberofDE)
            { 
                if (strcmp(tempBuffer[j].name, tokenArray[i]) == 0)
                {
                    // works up to here 100%

                    // this will update and it's to grab the final index
                    // index locaiton
                
                    location = tempBuffer[j].location;
                    globalTemp.dirPtr = tempBuffer[0].location;
                    numberofDE = tempBuffer[j].numOfDE;

                    // once the comparison is found, we read in the next location
                    LBAread(tempBuffer, blocksNeededForDir(numberofDE), location);
                    // free(tempBuffer);
                    // tempBuffer = NULL;
                    break;
                }
                j++;
                // grabbing result
                if (i == tokenIndex - 1) //This executes on final token
                {
                    //("grabbing result, setting globalTempt index to %d\n", j);
                    globalTemp.index = j;
                    
                }

                // in the case that we loop through the entire directory entries
                if (j == numberofDE - 1)
                {
                    if (i > 0)
                    {
                        globalTemp.dirPtr = tempBuffer[0].location;
                    }
                    else{
                    // This condition triggers only when directory is missing AT ROOT
                        
                        globalTemp.dirPtr = vcb->locOfRoot;
                    }
                    globalTemp.index = -1;
                    result.dirPtr = globalTemp.dirPtr;
                    result.index = globalTemp.index;
                    strcpy(result.lastArg, globalTemp.lastArg);
                    
                    return result;
                }

            }
      
        }
        
        //This executes when result is found
        result.dirPtr = globalTemp.dirPtr;
        result.index = globalTemp.index;
        strcpy(result.lastArg, globalTemp.lastArg);

        if (location == vcb->locOfRoot){
            result.dirPtr = vcb->locOfRoot;
            result.index = 0;
            strcpy(result.lastArg, "/");
        }
       
        

        return result;
    }
   
}



int fs_isFile(char *filename)
{
    // run parsepath to get a struct
    // so the tempLastArg is the current folder

    // parsepath will determine if its in the same folder or absolute

    struct fdPathResult tempPath = parsedPath(filename);

    LBAread(tempBuffer, blocksNeededForDir(MAXDE), tempPath.dirPtr);

    for (size_t i = 0; i < MAXDE; i++)
    {
        if (strcmp(tempBuffer[i].name, tempPath.lastArg) == 0)
        {
            if (tempBuffer[i].fileType == FT_REGFILE)
            {
                return 1;
            }
        }
    }

    return 0;
}

int fs_isDir(char *pathname)
{
    // run parsepath we get a struct to confirm it is a path as welll as location and stuff
    struct fdPathResult tempPath = parsedPath(pathname);
    // dirPtr, index, lastArg
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), tempPath.dirPtr);

    // iterate through tempBuffer to strcpy(tempbuffer[i].name, lastArg) == 0
    for (size_t i = 0; i < MAXDE; i++)
    {
        // check in the same folder and see if the file name passed in exists in the
        // current directory.
        if (strcmp(tempBuffer[i].name, tempPath.lastArg) == 0)
        {
            if (tempBuffer[i].fileType == FT_DIRECTORY)
            {
                return 1;
            }
        }
        // in the case that i is MAXDE and tempBuffer[i].name does not exist
        // return 0
        if (i == MAXDE - 1)
        {
          
            return 0;
        }
    }
    return 0;
}



char *fs_getcwd(char *pathname, size_t size)
{
   
    if (strlen(globalPath) > size){
        return NULL;
    }

    // this causes a stack smashing detection crash
    strncpy(pathname, globalPath, size);
    return globalPath;
}

// pathReconstruction checks if the global path contains . or ..
// if the path has those tokens, then this function removes the . and ..
// and corrects the global path
void fs_pathReconstruction (){
    // tokenizes path into tokens
    char *tokenArray[50];     // array of names to be tokenized
    const char s[2] = "/";    // delimiter
    int tokenIndex = 0;       // counter for number of tokens
    char str[strlen(globalPath)];   // declare a string, str to be read of size strlen(path)
    strcpy(str, globalPath);        // copy path into str
    
    // loop to tokenize values
    char *token = strtok(str, s);

    // tokenizes the values and inserts into tokenArray

    while (token != NULL)
    {
        tokenArray[tokenIndex++] = token;
        token = strtok(NULL, s);
    }

    char finalPath[MAXLENGTH] = "";
    char *finalPathArray[50]; // for edge cases of . and ..
    bool tokenFlag = 0;       // flag for . and ..
   

    for (size_t i = 0; i < tokenIndex; i++)
    {
        if ( (strcmp(tokenArray[i],".") == 0)  || (strcmp(tokenArray[i],"..") == 0)){
            tokenFlag = 1;
        }
    }
    
    // if there are . or .. we want to remove that from the global path
    if(tokenIndex ==1 && strcmp(tokenArray[0],"..")==0){
        strcpy(globalPath,"/"); 
    }

    
    /*
    The following creates a final path that removes all . and ..
    First there is a flag that runs the loop if there exists . or .. in the path
    Then if the token is a word, it adds the word to the finalPathArray
    if the token is . then nothing is added to the finalPathArray
    if the token is .. then a word is removed from the finalPathArray
    */
    if(tokenFlag == 1 && tokenIndex > 1){
        int counter = 0;
        for (size_t i = 0; i < tokenIndex; i++)
        {
        if ((strcmp(tokenArray[i], ".") == 0))
        {
            // do nothing
        }
      
        else if ((strcmp(tokenArray[i], "..") == 0))
        {   
            // reduce counter so we can remove item in array
            finalPathArray[counter] = "";
            counter--;
            if(counter == 0){
                strcat(finalPath, "/");
              
            }    
        }
        else
        {   
            // add it to temp array
            finalPathArray[counter] = tokenArray[i];
            counter++;
        }
        }
        
        for (size_t i = 0; i < counter; i++){
            strcat(finalPath, "/");
            strcat(finalPath, finalPathArray[i]);
        }
        
        strcpy(globalPath, finalPath);
    }
}

int fs_setcwd(char *pathname)
{   
    struct fdPathResult path = parsedPath(pathname);

    // check if isfile
    int isFile = fs_isFile(pathname);
    if (isFile == 1 ){
        return -1;
    }

    if (path.index == -1)
    {
        return -1;
    }
    else
    {   
        // case that global path is just "/"
        if(strlen(globalPath) == 1){
            // concatenate to globalPath
            strcat(globalPath, pathname);
            // copy concatenated global path to pathname buffer
            strcpy(pathname, globalPath);
        } 
    
        else {
            // same as above except if the global path starts with root
            strcat(globalPath, "/");
            strcat(globalPath, pathname);
            strcpy(pathname, globalPath);
      }  
        fs_pathReconstruction();
        return 0;
    
}
}


// makedir
int fs_mkdir(const char *pathname, mode_t mode)
{
    /*
    1 grab current directory
    2 parsePath
    3 LBAread
    4 iterate through to find name empty
    5 set as folder
    6 find space in free space map
    7 populate space with DEs
    8 return 1
    */
    struct fdPathResult path = parsedPath(pathname);

    // if the file already exists we dont need to make another
    if (path.index != -1)
    {
        printf("file exists\n");
        return -1;
    }
    

    // we have the pointer to the directory of where we want
    // to make the path.lastArg
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), path.dirPtr);


    int i = 2; // starting dir index of NOT "." or ".."
    while (i < MAXDE)
    {
        if (strcmp(tempBuffer[i].name, "") == 0)
        { // Upon finding first available DE slot
            // Prepare freespace
            size_t fssize = getFreespaceSize(vcb->numBlocks, vcb->blockSize);
            // uint8_t *freeSpaceMap = malloc(fssize);
            LBAread(freeSpaceMap, 5, vcb->locOfFreespace);
            int locOfNewDir = allocContBlocks(freeSpaceMap, fssize, blocksNeededForDir(MAXDE));

            // Prepare DE of new directory
            strcpy(tempBuffer[i].name, path.lastArg);
            tempBuffer[i].size = MAXDE * sizeof(DirectoryEntry);
            tempBuffer[i].fileType = FT_DIRECTORY;
            tempBuffer[i].numOfDE = MAXDE;
            tempBuffer[i].location = locOfNewDir;

            // Prepare the new directory itself

            //DirectoryEntry newDir[MAXDE];
            DirectoryEntry* newDir = calloc(blocksNeededForDir(MAXDE), vcb->blockSize);                     

            // initialize each directory entry of NEW DIR to be in a known free state
            for (int j = 0; j < MAXDE; j++)
            {
                strcpy(newDir[j].name, "");
            }
            // set the dot
            strcpy(newDir[0].name, ".");
            newDir[0].fileType = FT_DIRECTORY;
            newDir[0].location = locOfNewDir;
            newDir[0].numOfDE = MAXDE;
            // set the dot dot
            strcpy(newDir[1].name, "..");
            newDir[1].fileType = FT_DIRECTORY;
            newDir[1].location = path.dirPtr;
            newDir[1].numOfDE = tempBuffer[0].numOfDE;

            LBAwrite(tempBuffer, blocksNeededForDir(MAXDE), path.dirPtr);
            LBAwrite(newDir, blocksNeededForDir(MAXDE), locOfNewDir);
            int writeReturn;
            if (writeReturn = LBAwrite(freeSpaceMap, 5, 1) != 5){
                printf("Error Writing with LBAwrite, exiting program\n");
                exit(-1);
            }
            free(newDir);
            return locOfNewDir;
        }
        i++;
    }
    printf("Cannot make new dir, parent dir is full\n");
    return -1;
}

//
fdDir *fs_opendir(const char *pathname)
{
    // 1. parse the pathname, make sure path is valid and find the last element
    struct fdPathResult tempPath = parsedPath(pathname);

    // 2. check the last element to see if it is a directory
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), tempPath.dirPtr);

    //      a: yes if last Arg type  IS directory
    //      b: no -> fail return null it is not a directory
    if (tempBuffer[tempPath.index].fileType != FT_DIRECTORY)
    {
        return NULL;
    }

    // 3.  Load this directory
    
    // load directory into starting location for LBAread
    fdDir* fd = malloc(sizeof(fdDir));
    fd->directoryStartLocation = tempBuffer[tempPath.index].location;
    
    
    fd->dirEntryPosition = 0;
    fd->dirp_fs = malloc(sizeof(fs_diriteminfo));
    return fd;
}

// takes a pointer and returns a pointer to fs_diriteminfo struct
// fs_diriteminfo contains d_name, filetType, d_reclen
// this is just do a for loop
// returns a pointer to fs_diriteminfo struct
// return null when Error occurs

fs_diriteminfo *fs_readdir(fdDir *fd){
    // start from where we last left off, which was position 0
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), fd->directoryStartLocation);

    for(int i = fd->dirEntryPosition ; i < MAXDE; i++){
        //if this directory is used,
        //if DirectoryEntryUsed(dirp->dirp[i]){
        if (strcmp(tempBuffer[i].name, "") != 0){

            // ii fs_diriteminfo
            // copy the name from our directory entry to the struct
            // NEED TO FIX THIS
            strcpy(fd->dirp_fs->d_name, tempBuffer[i].name);

            // copy the fileType over to struct
            fd->dirp_fs->fileType = tempBuffer[i].fileType;

            // iterate the directory entry position to read the next slot
            fd->dirEntryPosition = i+1;

            //return fd->dirp_fs;
            return fd->dirp_fs;

        }
    }
    return NULL;

}

// free up memory here  we allocated in the open
int fs_closedir(fdDir *fd)
{
    free(fd->dirp_fs);
    free(fd);
    fd = NULL;
}

int fs_stat(const char *path, struct fs_stat *buf){
    struct fdPathResult result = parsedPath(path);
    if (result.index == -1){
        return -1;
    }
    LBAread(tempBuffer, blocksNeededForDir(MAXDE), result.dirPtr);
    buf->st_size = tempBuffer[result.index].size;
    buf->st_blocks = (tempBuffer[result.index].size + (vcb->blockSize - 1))/vcb->blockSize;
    return 0;
}
