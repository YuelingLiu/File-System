#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

void setBitOne(uint8_t *freeSpaceMap, int i);

void setBitZero(uint8_t *freeSpaceMap, int i);

bool getBit(uint8_t *freeSpaceMap, int i);

int getFreespaceSize(int numberOfBlocks, int blockSize);

// allocate contiguous blocks of free space for directories/files
int allocContBlocks(uint8_t *freeSpaceMap, size_t fssize, int num);

extern uint8_t *freeSpaceMap;

