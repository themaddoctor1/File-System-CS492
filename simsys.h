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
int  enoughMemFor(long);

/**
 * Gets a relative node in the tree structure.
 * tree - A subtree known to be a child of root.
 * path - A tokenized path between tree and the
 *        destination.
 */
DirTree getRelTree(DirTree, char**);

#endif
