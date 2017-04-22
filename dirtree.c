#include "linkedlist.h"
#include "dirtree.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct filedata {
    long size;
    LList blocks;
}; typedef struct filedata* FileData;

struct dirdata {
    LList files;
}; typedef struct filedata* DirData;

struct dirtree {
    /* Is the node a file or directory */
    int is_file;

    /* The name of the node */
    char *name;

    /* The parent directory */
    DirTree parent_dir;

    union {
        struct filedata file_dta;
        struct dirdata  dir_dta;
    };
};


/**
 * Creates a directory node. Duplicates the name w/ strdup().
 */
 
DirTree makeDirTree(char *name, int is_file) {
    DirTree node = (DirTree) malloc(sizeof(struct dirtree));
    node->name = strdup(name);
    node->is_file = is_file;

    if (is_file) {
        node->file_dta.size = 0;
    } else {
        node->dir_dta.files = makeLL();

        /* Default case: node is its own parent */
        node->parent_dir = node;
    }

    return node;
}

/**
 * Gets the directory node associated with the given path.
 * path - The tokenized path
 *
 * return - The requested node, or NULL if it doesn't exist.
 */
DirTree getDirSubtree(DirTree tree, char *path[]) {
    int i;

    if (!path || !path[0])
        return tree; /* Found the file */

    else if (tree->is_file)
        return NULL; /* Files do not have subdirectories. */

    else if (!strcmp(path[0], "."))
        return getDirSubtree(tree, &path[1]); /* Stay in current dir */

    else if (!strcmp(path[0], ".."))
        return getDirSubtree(tree->parent_dir, &path[1]); /* Go back one directory */
    
    /* Search the subfiles for the next recursive step */
    for (i = sizeOfLL(tree->dir_dta.files) - 1; i >= 0; i--) {
        DirTree child = (DirTree) getFromLL(tree->dir_dta.files, i);

        if (!strcmp(path[0], child->name)) {
            /* The child is should be searched through. */
            return getDirSubtree(child, &path[1]);
        }
    }

    return NULL;
}

int addNodeToTree(DirTree tree, char *path[], int is_file) {
    int i;
    DirTree tgtDir;
    char **subpath;
    char *filename;
    
    /* Make memory space for subpath */
    i = 0;
    while (path[i]) i++;
    subpath = (char**) malloc(i * sizeof(char*));

    /* Get the filename */
    filename = path[--i];
    
    /* Build the subpath */
    subpath[i--] = NULL;
    for (; i >= 0; i--)
        subpath[i] = path[i];
    
    /* Get the destination */
    tgtDir = getDirSubtree(tree, subpath);
    free(subpath);

    if (tgtDir == NULL) {
        /* Directory not found */
        return 1;
    } else if (tgtDir->is_file) {
        /* Cannot add node to file */
        return 2;
    } else {
        /* Make the file */
        DirTree file = makeDirTree(filename, is_file);

        /* Set the parent directory */
        file->parent_dir = tgtDir;

        /* Add to the file list */
        addToLL(tgtDir->dir_dta.files, 0, file);
        
        /* No error */
        return 0;
    }

}

int addDirToTree(DirTree tree, char *path[]) {
    return addNodeToTree(tree, path, 0);
}

int addFileToTree(DirTree tree, char *path[]) {
    return addNodeToTree(tree, path, 1);
}



long filesizeOfDirTree(DirTree tree, char *path[]) {
    if (!tree)
        return 0;
    else if (!path || !path[0]) {
        /* Current node is the root. */

        long size = 0; /* Size of the directory node */
        
        /* File check */
        if (tree->is_file)
            return tree->file_dta.size;
        
        /* Node is a directory, so it is a combo of sizes. */
        for (int i = sizeOfLL(tree->dir_dta.files) - 1; i >= 0; i--)
            size += filesizeOfDirTree((DirTree) getFromLL(tree->dir_dta.files, i), NULL);

        return size;
    } else {
        /* Find subtree and get size */
        return filesizeOfDirTree(getDirSubtree(tree, path), NULL);
    }
}

long numFilesInTreeDir(DirTree tree, char *dir[], int rec) {
    if (!tree)
        return 0;
    else if (dir && dir[0]) /* Recursive initial condition */
        return numFilesInTreeDir(getDirSubtree(tree, dir), NULL, rec);
    else {
        int i;
        long count = 0;
        
        /* Check each subfile */
        for (i = sizeOfLL(tree->dir_dta.files) - 1; i >= 0; i--) {
            DirTree sub = getFromLL(tree->dir_dta.files, i);

            /* Only count directories if recursively checking */
            if (sub->is_file)
                count += sub->file_dta.size;
            else
                count += rec ? numFilesInTreeDir(sub, NULL, rec) : 0;
        }

        return count;
    }
}

long treeFileSize(DirTree tree, char *path[]) {
    DirTree file = path ? getDirSubtree(tree, path) : tree;

    if (file->is_file)
        return file->file_dta.size;
    else
        return 0;
}


int rmfileFromTree(DirTree tree, char *path[]) {
   if (!tree)
        return 1;
    else if (!path || !path[0]) {
        /* This file is the target */
        DirTree parent = tree->parent_dir;
        
        if (!(tree->is_file)) {
            /* Node is not for a file */
            return 2;
        }
        
        /* Remove the linking from the tree */
        if (parent) {
            remFromLL(parent->dir_dta.files, indexOfLL(parent->dir_dta.files, tree));
            tree->parent_dir = NULL;
        }

        /* Handle the destruction of the file */
        while (sizeOfLL(tree->file_dta.blocks))
            remFromLL(tree->file_dta.blocks, 0);

        /* Zero the file data */
        tree->file_dta.size = 0;

        free(tree->file_dta.blocks);
        tree->file_dta.blocks = NULL;

        free(tree);

        return 0;

    } else
        return rmfileFromTree(getDirSubtree(tree, path), NULL);
}

int rmdirFromTree(DirTree tree, char *path[]) {
   if (!tree)
        return 1;
    else if (!path || !path[0]) {
        /* This file is the target */
        DirTree parent = tree->parent_dir;
        
        if (tree->is_file) {
            /* Node is a file */
            return 2;
        } else if (sizeOfLL(tree->file_dta.blocks)) {
            /* Do not allow a directory with contents to be destroyed */
            return 3;
        }
        
        if (parent) {
            /* Remove linking with parent */
            remFromLL(parent->dir_dta.files, indexOfLL(parent->dir_dta.files, tree));
            tree->parent_dir = NULL;
        }

        /* Zero the file data */
        free(tree->dir_dta.files);
        tree->dir_dta.files = NULL;

        free(tree);

        return 0;

    } else
        return rmfileFromTree(getDirSubtree(tree, path), NULL);
}

int isTreeFile(DirTree tree) {
    return tree->is_file;
}

char* getTreeFilename(DirTree tree) {
    return tree->name;
}

LList getDirTreeChildren(DirTree tree) {
    LList list = makeLL();
    int i;

    if (tree->is_file)
        return list;
    
    /* Add each item in alphabetical order */
    for (i = sizeOfLL(tree->dir_dta.files) - 1; i >= 0; i--) {
        DirTree elem = (DirTree) getFromLL(tree->dir_dta.files, i);
        
        int j;
        for (j = sizeOfLL(list) - 1; j >= 0; j--) {
            DirTree tst = (DirTree) getFromLL(list, j);
            if (strcmp(tst->name, elem->name) < 0) {
                break;
            }
        }

        addToLL(list, j+1, elem);

    }

    return list;

}



