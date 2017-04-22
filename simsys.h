#ifndef _SIMSYS_H_
#define _SIMSYS_H_

#include "dirtree.h"

void init_filesystem(long blks, long size);

DirTree getRootNode();
DirTree getWorkDirNode();

void setWorkDirNode(DirTree);

long blockSize();

/**
 * Requests a number of blocks, and returns the ids of
 * each successfully acquired block. Given n, at most
 * n blocks will be acquired.
 */

void freeBlock(long);
long allocBlock();

#endif
