#include "cmds.h"
#include "dirtree.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>


int cmd_defrag1(char *argv[]) {
    
    DirTree root = getRootNode();
    
    LList F = makeLL();

    long lo;
    
    /* Reorganize each file's blocks */
    appendToLL(F, root);
    while (!isEmptyLL(F)) {
        
        DirTree f = (DirTree) remFromLL(F, 0);

        if (isTreeFile(f)) {
            
            /* Get block information */
            LList blocks = getTreeFileBlocks(f);
            int num_blks = sizeOfLL(blocks);

            /* Holds the blocks, which will be resorted */
            long *blks = (long*) malloc(num_blks * sizeof(long));
            int i;
            
            /* Iterator */
            LLiter iter;
            
            /* Dequeue each block number */
            iter = makeLLiter(blocks);
            for (i = 0; iterHasNextLL(iter); i++)
                blks[i] = *((long*) iterNextLL(iter));

            free(blocks);
            disposeIterLL(iter);

            /* Sort the blocks */
            mergesort_longs(blks, 0, num_blks);
            
            /* Remove the old listing */
            for (i = 0; i < num_blks; i++)
                releaseMemoryBlock(f);
            
            /* Replace with a new one */
            for (i = 0; i < num_blks; i++)
                assignMemoryBlock(f, blks[i]);

            free(blks);

        } else {
            /* f is a directory; apply BFS condition */
            LList children = getDirTreeChildren(f, 0);

            while (!isEmptyLL(children))
                appendToLL(F, remFromLL(children, 0));

            free(children);
        }
        
    }

    lo = nextBlock();
    
    /* Continuously push the blocks forward */
    while (numSectors() > 1) {
        appendToLL(F, root);

        while (!isEmptyLL(F)) {
        
            DirTree f = (DirTree) remFromLL(F, 0);

            if (isTreeFile(f)) {
                
                /* Get block information */
                LList blocks = getTreeFileBlocks(f);
                int num_blks = sizeOfLL(blocks);

                /* Holds the blocks, which will be resorted */
                long *blks = (long*) malloc(num_blks * sizeof(long));
                int i;
                
                /* Iterator */
                LLiter iter;
                
                /* Dequeue each block number */
                iter = makeLLiter(blocks);
                for (i = 0; iterHasNextLL(iter); i++)
                    blks[i] = *((long*) iterNextLL(iter));

                free(blocks);
                disposeIterLL(iter);
                
                /* Dump the old listing */
                for (i = 0; i < num_blks; i++)
                    releaseMemoryBlock(f);

                for (i = 0; i < num_blks; i++) {
                    if (blks[i] > lo) {
                        freeBlock(blks[i]);
                        blks[i] = allocBlock();
                        lo = nextBlock();
                    }

                    assignMemoryBlock(f, blks[i]);
                }

                free(blks);

            } else {
                /* f is a directory; apply BFS condition */
                LList children = getDirTreeChildren(f, 0);

                while (!isEmptyLL(children))
                    appendToLL(F, remFromLL(children, 0));

                free(children);
            }
            
        }
        
    }

    return 0;
    

}

int cmd_defrag2(char *argv[]) {
    
    DirTree root = getRootNode();
    
    LList F = makeLL();
    LList scratch = makeLL();

    long lo = nextBlock();
    long blks_allocd = blocksAllocated();

    /* Continuously push the blocks forward */
    while (numSectors() > 1) {
        appendToLL(F, root);

        while (!isEmptyLL(F)) {
        
            DirTree f = (DirTree) remFromLL(F, 0);

            if (isTreeFile(f)) {
                
                /* Get block information */
                LList blocks = getTreeFileBlocks(f);
                int num_blks = sizeOfLL(blocks);

                /* Holds the blocks, which will be resorted */
                long *blks = (long*) malloc(num_blks * sizeof(long));
                int i;
                
                /* Iterator */
                LLiter iter;
                
                /* Dequeue each block number */
                iter = makeLLiter(blocks);
                for (i = 0; iterHasNextLL(iter); i++)
                    blks[i] = *((long*) iterNextLL(iter));

                free(blocks);
                disposeIterLL(iter);
                
                /* Dump the old listing */
                for (i = 0; i < num_blks; i++)
                    releaseMemoryBlock(f);

                for (i = 0; i < num_blks; i++) {
                    if (blks[i] > lo) {
                        freeBlock(blks[i]);
                        blks[i] = allocBlock();
                        lo = nextBlock();
                    }

                    assignMemoryBlock(f, blks[i]);
                }

                free(blks);

            } else {
                /* f is a directory; apply BFS condition */
                LList children = getDirTreeChildren(f, 0);

                while (!isEmptyLL(children))
                    appendToLL(F, remFromLL(children, 0));

                free(children);
            }
            
        }
        
    }
    
    /* Build F so that it lists every file */
    appendToLL(scratch, root);
    while (!isEmptyLL(scratch)) {
        DirTree f = (DirTree) remFromLL(scratch, 0);
        
        if (isTreeFile(f))
            appendToLL(F, f);
        else {
            LList children = getDirTreeChildren(f, 0);

            while (!isEmptyLL(children))
                appendToLL(scratch, remFromLL(children, 0));

            free(children);
        }
            
    }
    free(scratch);
    
    /* Make all blocks sequential */
    lo = 0;
    while (lo < blks_allocd) {
        int fd = -1;
        int i;
        DirTree f;
        LLiter A = makeLLiter(F);
        
        LList blkLL;
        long **blocks;
        int numBlks;
        
        /* Gets the file descriptor of the file */
        while (iterHasNextLL(A)) {
            LList blks;
            fd++;

            f = (DirTree) iterNextLL(A);
            blks = getTreeFileBlocks(f);

            while (!isEmptyLL(blks)) {
                if (*((long*) remFromLL(blks, 0)) == lo) {
                    /* The current f is the item we want */
                    A = NULL;
                }
            }
            free(blks);

        }
        disposeIterLL(A);
        
        /* We need the list of blocks given to the file */
        blkLL = (LList) getTreeFileBlocks(f);
        numBlks = sizeOfLL(blkLL);
        blocks = (long**) malloc(numBlks * sizeof(long*));

        i = 0;
        while (!isEmptyLL(blkLL)) {
            blocks[i] = ((long*) remFromLL(blkLL, 0));
            i++;
        }
        free(blkLL);
        
        /* For each value in A (other than lo), get a lower block number */
        A = makeLLiter(F);
        
        /* Get the first block that needs to be swapped */
        i = 0;
        while (i < numBlks && blocks[i][0] < lo + numBlks) i++;

        while (iterHasNextLL(A) && i < numBlks) {
            DirTree g = iterNextLL(A);
            
            if (f == g)
                continue; /* No reason to swap with self */
            
            /* Check each block, and replace if needed */
            scratch = getTreeFileBlocks(g);
            while (!isEmptyLL(scratch)) {
                /* blk points to one of g's allocated blocks */
                long *blk = (long*) remFromLL(scratch, 0);
                
                /* Don't continue if i is out of bounds */
                if (i < numBlks && *blk < lo + numBlks) {
                    /* Replace the block at i */
                    long tmp = blocks[i][0];
                    blocks[i][0] = *blk;
                    *blk = tmp;

                    /* Find a new value to replace */
                    do {
                        i++;
                    } while (i < numBlks && *(blocks[i]) >= lo + numBlks);
                }
            }
            free(scratch);

        }
        disposeIterLL(A);

        /* Remove f from F; maintains that everything before lo is handled */
        remFromLL(F, fd);

        /* Get rid of extra memory */
        free(blocks);
        lo += numBlks;
    }

    /* Reorganize each file's blocks */
    appendToLL(F, root);
    while (!isEmptyLL(F)) {
        
        DirTree f = (DirTree) remFromLL(F, 0);

        if (isTreeFile(f)) {
            
            /* Get block information */
            LList blocks = getTreeFileBlocks(f);
            int num_blks = sizeOfLL(blocks);

            /* Holds the blocks, which will be resorted */
            long *blks = (long*) malloc(num_blks * sizeof(long));
            int i;
            
            /* Iterator */
            LLiter iter;
            
            /* Dequeue each block number */
            iter = makeLLiter(blocks);
            for (i = 0; iterHasNextLL(iter); i++)
                blks[i] = *((long*) iterNextLL(iter));

            free(blocks);
            disposeIterLL(iter);

            /* Sort the blocks */
            mergesort_longs(blks, 0, num_blks);
            
            /* Remove the old listing */
            for (i = 0; i < num_blks; i++)
                releaseMemoryBlock(f);
            
            /* Replace with a new one */
            for (i = 0; i < num_blks; i++)
                assignMemoryBlock(f, blks[i]);

            free(blks);

        } else {
            /* f is a directory; apply BFS condition */
            LList children = getDirTreeChildren(f, 0);

            while (!isEmptyLL(children))
                appendToLL(F, remFromLL(children, 0));

            free(children);
        }
        
    }

    free(F);

    return 0;
    

}


int cmd_defrag(char *argv[]) {
    if (!argv[1])
        cmd_defrag1(NULL);
    else if (!strcmp(argv[1], "1"))
        cmd_defrag1(NULL);
    else if (!strcmp(argv[1], "2"))
        cmd_defrag2(NULL);
    else {
        printf("defrag: routine '%s' is not defined\n", argv[1]);
        return 1;
    }

    return 0;
}




