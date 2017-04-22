#include "simsys.h"

#include <stdlib.h>
#include <stdio.h>

long BLOCK_SIZE;
long NUM_BLOCKS;

DirTree ROOT;
DirTree WORK_DIR;

/**
 * The list of allocated blocks.
 * Invariant - For any index i = 2n,
 *             i is the start of a memory
 *             block, and i+1 is the end of
 *             the same block.
 */
LList MEM_ALLOC;

void init_filesystem(long blks, long size) {

    BLOCK_SIZE = size / blks;
    NUM_BLOCKS = blks;

    ROOT = makeDirTree("", 0);

    WORK_DIR = ROOT;

    MEM_ALLOC = makeLL();
}

DirTree getRootNode() {
    return ROOT;
}

DirTree getWorkDirNode() {
    return WORK_DIR;
}

void setWorkDirNode(DirTree node) {
    WORK_DIR = node;
}

long blockSize() {
    return BLOCK_SIZE;
}

void freeBlock(long blk) {
    int sectors = sizeOfLL(MEM_ALLOC);
    long lo, hi;
    int i;
    
    /* End result: The ith block contains block blk. */
    for (i = 0; i < sectors; i++) {
        if (   *((long*) getFromLL(MEM_ALLOC, 2*i)) <= blk
            && *((long*) getFromLL(MEM_ALLOC, 2*i+1)) > blk)
                break;
    }

    if (i == sectors)
        return;
    
    lo = *((long*) getFromLL(MEM_ALLOC, 2*i));
    hi = *((long*) getFromLL(MEM_ALLOC, 2*i+1));
    
    if (lo == hi-1) {
        free(getFromLL(MEM_ALLOC, 2*i));
        free(getFromLL(MEM_ALLOC, 2*i));
    } else if (lo == blk) {
        *((long*) getFromLL(MEM_ALLOC, 2*i)) += 1;
    } else if (hi-1 == blk) {
        *((long*) getFromLL(MEM_ALLOC, 2*i+1)) -= 1;
    } else {

        long *mLo = (long*) malloc(sizeof(long));
        long *mHi = (long*) malloc(sizeof(long));

        *mLo = blk;
        *mHi = blk+1;
        
        /* Create a one block gap in the memory */
        addToLL(MEM_ALLOC, 2*i+1, mLo);
        addToLL(MEM_ALLOC, 2*i+2, mHi);
    }

}

long allocBlock() {
    
    long sectors = sizeOfLL(MEM_ALLOC) / 2;
    long *tmp;

    if (sectors == 0) {
        /* No memory allocated */
        tmp = (long*) malloc(sizeof(long));
        *tmp = 0;
        addToLL(MEM_ALLOC, 0, tmp);
        
        tmp = (long*) malloc(sizeof(long));
        *tmp = 1;
        addToLL(MEM_ALLOC, 1, tmp);

        return 0;
        
    } else {
        long min_free = 0;
        long max_free = *((long*) getFromLL(MEM_ALLOC, 0));
        
        if (min_free == max_free) {
            /* Can't place in front */
            min_free = *((long*) getFromLL(MEM_ALLOC, 1));
            max_free = sectors == 1
                                  ? NUM_BLOCKS
                                  : *((long*) getFromLL(MEM_ALLOC, 3));
            
            if (min_free == max_free) {
                /* No available memory */
                return -1;
            } else if (min_free == max_free - 1) {
                /* Closes a one-byte gap in memory */
                free(remFromLL(MEM_ALLOC, 1));
                free(remFromLL(MEM_ALLOC, 1));
            } else {
                *((long*) getFromLL(MEM_ALLOC, 1)) += 1;
            }

            return min_free;

        } else if (min_free == max_free - 1) {
            /* Close a one-block frag in the front */
            *((long*) getFromLL(MEM_ALLOC, 0)) = 0;

            return 0;
        } else {
            /* Memory in non-bordering space */
            tmp = (long*) malloc(sizeof(long));
            *tmp = 0;
            addToLL(MEM_ALLOC, 0, tmp);
            
            tmp = (long*) malloc(sizeof(long));
            *tmp = 1;
            addToLL(MEM_ALLOC, 1, tmp);

            return 0;
        }


    }

}
