#include "DE.h"

int blocksNeededForDir(int maxde){
    int bytesNeeded = maxde * sizeof(DirectoryEntry);
    int blocksNeeded = (bytesNeeded + (vcb->blockSize - 1)) / vcb->blockSize;
}