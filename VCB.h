

typedef struct VCB {
    // Dictate the total number of blocks in the volume
    int numBlocks;

    // Dictate how many bytes correspond to a single block
    int blockSize;

    // "Pointer" to first block of freespace bitmap
    int locOfFreespace;

    // "Pointer" to the first block of root directory
    int locOfRoot;

    // unique magic number to identify if the volume belongs to us
    long signature;
} VCB;

//Ensuring the VCB is accessible from anywhere
extern VCB* vcb;
