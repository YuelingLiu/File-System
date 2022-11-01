

typedef struct DirectoryEntry {
    // the name of the entry that is unique to that file, and is used for lookup
    char name[256];

    // the size of the file so we know how far to read up to
    long size;

    // Dictate the total number of directory entries we want for a directory.
    int numOfDE;

    // Dictate the bytes we need multiply the size of your directory entry
    // by the number of entries
    int bytesNeeded;

    // Dictate the current time
    time_t timeStamp;

    // SUBJECT TO CHANGE: for now, going forward using a file allocation method
    // that requires a “pointer” to the starting block of the file.
    long location;

} DirectoryEntry;