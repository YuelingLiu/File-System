#include "freespace.h"

void setBitOne(uint8_t *freeSpaceMap, int i){
    freeSpaceMap[i >> 3] |= (1 << (i & 0x7));
}

void setBitZero(uint8_t *freeSpaceMap, int i){
    freeSpaceMap[i >> 3] &= (0 << (i & 0x7));
}

bool getBit(uint8_t *freeSpaceMap, int i){
    return freeSpaceMap[i >> 3] & (1 << (i & 0x7));
}

// passing the value directly so we don't have to have ugly code
int getFreespaceSize(int numberOfBlocks, int blockSize){
    int bytesNeeded = (numberOfBlocks + 7) / 8; // 2442.25
    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize; // 5
    return (blocksNeeded * blockSize); // 2560
}

// allocate contiguous blocks of free space for directories/files
int allocContBlocks(uint8_t *freeSpaceMap, size_t fssize, int num){
    // count how many contiguous free blocks there are starting from the first free one
    int freeBlockCounter = 0;
    // amount of used blocks inside the byte before the first free one
    int firstBitOffset = 0;

    // read in one byte at a time (8 bits, 1 byte block)
    for (size_t byteIndex = 0; byteIndex < fssize; byteIndex++){
        // check to confirm that it's not all 1s
        // if all bytes are 1111111 then that means there is no space
        // if there are zeroes then we have to traverse to find the first zero
        if (freeSpaceMap[byteIndex] != 255){
            // traverse through the byte until we find the first zero
            while(getBit(freeSpaceMap, (byteIndex * 8) + firstBitOffset) == 1){
                firstBitOffset++;
            }
            // after we found the first zero, traverse until we reach amount requested
            // or encounter a 1
            while(getBit(freeSpaceMap, (byteIndex * 8) + firstBitOffset + freeBlockCounter) == 0){
                freeBlockCounter++;

                // once freeBlockCounter is equal to userInput, we have found the space
                // starting at (byteIndex * 8) + firstBitOffset;
                if (freeBlockCounter == num){
                    for (int i = (byteIndex * 8)+firstBitOffset; i < (byteIndex * 8)+firstBitOffset + freeBlockCounter; i++){
                        setBitOne(freeSpaceMap, i); // mark the bits as used
                    }
                    

                    return (byteIndex * 8) + firstBitOffset;
                }
            }
        }
        freeBlockCounter = 0;
        firstBitOffset = 0;
    }
    return -1;
}

int allocSingleBlock(uint8_t *freeSpaceMap, size_t fssize){
    // amount of used blocks inside the byte before the first free one
    int firstBitOffset = 0;
    // read in one byte at a time (8 bits, 1 byte block)
    for (size_t byteIndex = 0; byteIndex < fssize; byteIndex++){
        // check to confirm that it's not all 1s
        // if all bytes are 1111111 then that means there is no space
        // if there are zeroes then we have to traverse to find the first zero
        if (freeSpaceMap[byteIndex] != 255){
            // traverse through the byte until we find the first zero
            while(getBit(freeSpaceMap, (byteIndex * 8) + firstBitOffset) == 1){
                firstBitOffset++;
            }
            int i = (byteIndex * 8)+firstBitOffset;
            setBitOne(freeSpaceMap, i);
            return i;
        }
        firstBitOffset = 0;
    }
    return -1;
}