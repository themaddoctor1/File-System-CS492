#ifndef _DIRTREE_H_
#define _DIRTREE_H_

#include "linkedlist.h"

#include <time.h>

struct dirtree;
typedef struct dirtree* DirTree;

long BLOCK_SIZE;

DirTree makeDirTree(char *name, int is_file);

DirTree getDirSubtree(DirTree tree, char *path[]);

/**
 * Adds a directory path to the tree
 * path - The filepath, vectorized
 *
 * return - An nonzero error code if something went wrong:
 *          1 - File not found
 *          2 - Tried to add file to a file
 */
int addDirToTree(DirTree tree, char *path[]);

/**
 * Adds a file to the system.
 * path - The filepath, ending in the filename
 *
 * return - An nonzero error code if something went wrong:
 *          1 - File not found.
 *          2 - Tried to add file to a file
 */
int addFileToTree(DirTree tree, char *path[]);

/**
 * Removes a file from the system.
 * path - The filepath, ending in the filename.
 *
 * return - A nonzero error code if something went wrong:
 *          1 - File not found.
 *          2 - Tried to remove directory when file expected.
 */
int rmfileFromTree(DirTree tree, char *path[]);

/**
 * Removes a directory from the system.
 * path - The filepath, ending in the filename.
 *
 * return - A nonzero error code if something went wrong:
 *          1 - File not found.
 *          2 - Tried to remove file when directory expected.
 */
int rmdirFromTree(DirTree tree, char *path[]);

/* Statistical functions */

/**
 * Computes the size of a directory, in bytes.
 */
long filesizeOfDirTree(DirTree tree, char *path[]);

/**
 * Computes the number of files in the directory.
 */
long numFilesInTreeDir(DirTree tree, char *dir[], int recursive);

/**
 * Computes size of file, in bytes. If actually a directory, returns 0.
 */
long treeFileSize(DirTree tree, char *filepath[]);

/**
 * Whether or not a given tree root is a directory
 */
int isTreeFile(DirTree tree);

/**
 * The name of the given tree directory or file.
 */
char* getTreeFilename(DirTree tree);

/**
 * The children of the given tree root.
 */
LList getDirTreeChildren(DirTree tree);

char** pathVecOfTree(DirTree tree);

time_t getTreeTimestamp(DirTree);

void updateFileSize(DirTree, long);

/* Memory assignment functions */
void assignMemoryBlock(DirTree, long);
long releaseMemoryBlock(DirTree);

#endif
