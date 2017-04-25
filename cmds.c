#include "cmds.h"
#include "dirtree.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** str_to_vec(char *str, char split_c) {
    char** vec = (char**) malloc(sizeof(char*));
    char *next;
    int i, j, k;
    int size = 0;

    vec[0] = NULL;
    
    k = 0;
    while (1) {
        int m, n;
        char **tmp;
        
        for (m = 0, n = 0; str[k+n+m] != split_c && str[k+n+m]; n++) {
            /* Skip backslashes */
            if (str[k+n+m] == '\\'/* && str[k+n+m+1]*/)
                m++;
        }
        
        /* Make the new string */
        next = (char*) malloc((n+1) * sizeof(char));
        for (i = 0, j = 0; i < n; i++) {
            if (str[k+i+j] == '\\')
                j++;
            next[i] = str[k+i+j];
        }
        next[n] = '\0';

        vec[size] = next;

        /* Realloc */
        size++;
        tmp = (char**) realloc(vec, (size+1) * sizeof(char*));
        if (!tmp) {
            printf("Have to reallocate.\n");
            tmp = (char**) malloc((size+1) * sizeof(char*));
            memcpy(tmp, vec, size * sizeof(char*));
        }

        vec = tmp;
        vec[size] = NULL;
        
        /* Update k */
        if (str[k+m+n]) 
            k += n+m+1;
        else
            return vec;

    }

}

void free_str_vec(char **vec) {
    int i;
    for (i = 0; vec[i]; i++) {
        free(vec[i]);
        vec[i] = NULL;
    }
    free(vec);
}

void error_message(char *cmd, char *mssg) {
    printf("%s: Cannot perform operation: %s\n", cmd, mssg);
}

/* Print an ls -l esque line */
void printTreeNode(DirTree node, int fullpath, int details) {
    
    char *filename;
    char buff[32];

    int is_file;
    long filesize;

    if (!node)
        return;

    is_file = isTreeFile(node);
    filesize = treeFileSize(node, NULL);
    filename = getTreeFilename(node);
    
    if (details) {
        time_t time = getTreeTimestamp(node);
        struct tm* timestamp = localtime(&time);

        strftime(buff, 32*sizeof(char), "%b %d %H:%M", timestamp);
        printf("%-15s ", buff);
        printf("%-15ld ", filesize);

    }
    
    /* Blue bold for the files */
    if (!is_file)
        printf("\033[1m\033[34m");

    if (fullpath) {
        /* Show full path */
        char **path = pathVecOfTree(node);
        int i;
        
        /* Print each file layer */
        for (i = 0; path[i]; i++)
            printf("%s%s", path[i],
                path[i+1] ? "/" : ""
        );

        free_str_vec(path);

    } else
        printf("%s", filename);
    
    if (!is_file)
            printf("/");

    printf("\033[0m\n");
 
            /*
            int is_file = isTreeFile(file);
             
            printf("%s%s%s", 
                is_file ? "" : "\033[1m\033[34m",
                getTreeFilename(file),
                "\033[0m");
            
            *//* Prints only if the child is a file *//*
            if (is_file)
                printf(" - %ldB",treeFileSize(file, NULL));

            printf("\n");
            */   
}

/**
 * Runs a command.
 *
 * argv - The command vector to execute.
 */
void cmd_exec(char *argv[]) {
    SimCmd cmd;
    char *name = argv[0];
    
    if (!strcmp(name, "cd"))
        cmd = cmd_cd;
    else if (!strcmp(name, "ls"))
        cmd = cmd_ls;
    else if (!strcmp(name, "mkdir"))
        cmd = cmd_mkdir;
    else if (!strcmp(name, "create"))
        cmd = cmd_create;
    else if (!strcmp(name, "append"))
        cmd = cmd_append;
    else if (!strcmp(name, "remove"))
        cmd = cmd_remove;
    else if (!strcmp(name, "delete"))
        cmd = cmd_delete;
    else if (!strcmp(name, "exit"))
        cmd = cmd_exit;
    else if (!strcmp(name, "dir"))
        cmd = cmd_dir;
    else if (!strcmp(name, "prfiles"))
        cmd = cmd_prfiles;
    else if (!strcmp(name, "prdisks"))
        cmd = cmd_prdisks;
    else {
        printf("%s: command not found.\n", argv[0]);
        return;
    }

    cmd(argv);
}

int cmd_cd(char *argv[]) {
    char **dirtoks;
    
    if (!argv[1]) {
        /* Default is to go to root. */
        setWorkDirNode(getRootNode());
        return 0;
    } else if (argv[2]) {
        error_message("cd", "Too many arguments provided.");
        return 1;
    } else {
        DirTree tgt;

        /* Build a token vector */
        dirtoks = str_to_vec(argv[1], '/');

        /* Get the destination */
        tgt = getRelTree(getWorkDirNode(), dirtoks);

        /* Free the vector */
        free_str_vec(dirtoks);
        
        if (!tgt) {
            error_message("cd", "Directory not found.");
            return 1;
        } else if (isTreeFile(tgt)) {
            error_message("cd", "Target is not a directory.");
            return 1;
        } else {
            setWorkDirNode(tgt);
            return 0;
        }
    }
}

/**
 * Lists the contents of a directory.
 */
int cmd_ls(char *argv[]) {
    DirTree tgt;
    DirTree working_dir = getWorkDirNode();

    if (argv[1]) {
        char **dirtoks = str_to_vec(argv[1], '/');

        /* Adjust the node to print */
        tgt = getRelTree(working_dir, dirtoks);

        free_str_vec(dirtoks);
    } else
        tgt = working_dir;

    if (!tgt) {
        error_message("ls", "Directory not found.\n");
        return 1;
    } else if (isTreeFile(tgt)) {
        error_message("ls", "Target is not a directory.\n");
        return 1;
    } else {
        LList files = getDirTreeChildren(tgt);

        printf("total %i\n", sizeOfLL(files));
        
        /* Go through each file, removing from the list as it goes */
        while (!isEmptyLL(files)) {
            DirTree file = (DirTree) remFromLL(files, 0);

            printTreeNode(file, 0, 1);
        }
        
        /* Delete the list */
        free(files);
    }
    
    return 0;
}

/**
 * Creates a directory, or set of directories.
 */
int cmd_mkdir(char *argv[]) {
    
    if (!argv[1]) {
        error_message("mkdir", "No directory names provided.");
        return 1;
    } else {
        /* Get a list of all of the files in the directory */
        int errCode = 0;

        int i = 1;
        while (argv[i]) {
            /* Build a path */
            char **path = str_to_vec(argv[i], '/');
            char *dirnm;
            int j, k;
            DirTree tgtDir;
            LList files;
            
            /* Separate the target name and its path */
            for (k = 0; path[k]; k++);
            dirnm = path[k-1];
            path[k-1] = NULL;
            
            /* Get the potential parent node */
            tgtDir = getRelTree(getWorkDirNode(), path);

            if (!tgtDir) {
                /* The containing path does not exist. */
                printf("mkdir: Cannot make directory '%s': No such file or directory\n", argv[i]);
                i++;
                continue;
            }

            files = getDirTreeChildren(tgtDir);

            for (j = sizeOfLL(files) - 1; j >= 0; j--) {
                DirTree file = (DirTree) getFromLL(files, j);

                if (!strcmp(getTreeFilename(file), argv[i])) {
                    /* Don't make duplicate directories */
                    printf("mkdir: Cannot make directory '%s': Already exists\n", argv[i]);
                    errCode = 1;
                    break;
                }
            }

            path[k-1] = dirnm;
            
            if (j < 0) {
                /* Add the directory */
                addDirToTree(getWorkDirNode(), path);
            }

            /* Free used memory */
            free_str_vec(path);
            while (!isEmptyLL(files))
                remFromLL(files, 0);
            free(files);

            i++;
        }

        
        return errCode;
    }

}

/**
 * Creates a file in the file structure.
 */
int cmd_create(char *argv[]) {
    if (!argv[1]) {
        error_message("create", "No file names provided.");
        return 1;
    } else {
        /* Get a list of all of the files in the directory */
        int errCode = 0;

        int i = 1;
        while (argv[i]) {
            /* Build a path */
            char **path = str_to_vec(argv[i], '/');
            char *filenm;
            int j, k;
            DirTree tgtDir;
            LList files;
            
            /* Separate the target name and its path */
            for (k = 0; path[k]; k++);
            filenm = path[k-1];
            path[k-1] = NULL;
            
            /* Get the potential parent node */
            tgtDir = getRelTree(getWorkDirNode(), path);
            files = getDirTreeChildren(tgtDir);


            for (j = sizeOfLL(files) - 1; j >= 0; j--) {
                DirTree file = (DirTree) getFromLL(files, j);

                if (!strcmp(getTreeFilename(file), argv[i])) {
                    /* Don't make duplicate directories */
                    printf("create: Cannot make file %s: Already exists.\n", argv[i]);
                    errCode = 1;
                    break;
                }
            }

            path[k-1] = filenm;
            
            if (j < 0) {
                /* Add the directory */
                addFileToTree(getWorkDirNode(), path);
            }

            /* Free used memory */
            free_str_vec(path);
            while (!isEmptyLL(files))
                remFromLL(files, 0);
            free(files);

            i++;
        }

        
        return errCode;
    }
}

int cmd_append(char *argv[]) {
    error_message(argv[0], "Not yet implemented."); return 0;
}


int cmd_remove(char *argv[]) {
    error_message(argv[0], "Not yet implemented."); return 0;
}


int cmd_delete(char *argv[]) {
    if (!argv[1]) {
         printf("rm: missing operand\n");
         return 1;
    } else {
        int errCode = 0;
        int i = 1;
        
        /* TODO: Handle freeing memory on file deletion (requires implementing remove) */
        printf("\033[1m\033[33mWarning\033[0m: delete does not currently handle memory free for files\n");

        while (argv[i]) {
            char **path = str_to_vec(argv[i], '/');
            DirTree tgt = getRelTree(getWorkDirNode(), path);
            int n;

            if (!tgt) {
                errCode = 1;
                n = 1;
            } else if (isTreeFile(tgt)) {
                /* Handle file removal */
                n = rmfileFromTree(tgt, NULL);
            } else {
                /* Handle directory removal. */
                n = rmdirFromTree(tgt, NULL);
            }

            if (n) {
                errCode = 1;
                printf("delete: cannot delete '%s': No such file or directory", argv[i]);

            }

            i++;

        }

        return errCode;
    }
}


/**
 * Terminates the program.
 */
int cmd_exit(char *argv[]) {

    /* TODO: Handle freeing all memory */
    printf("\033[1m\033[33mWarning\033[0m: exit does not free used memory\n");
    
    if (argv[1])
        exit(atoi(argv[1]));
    else
        exit(0);

    return 0;

}

int cmd_dir(char *argv[]) {
    
    DirTree root;
    LList bfs_list = makeLL();
    
    /* Get the top directory of the BFS */
    if (!argv[1])
        root = getWorkDirNode();
    else {
        char **dirtoks = str_to_vec(argv[1], '/');
        root = getRelTree(getWorkDirNode(), dirtoks);
        free_str_vec(dirtoks);
    }
    
    appendToLL(bfs_list, root);

    while (!isEmptyLL(bfs_list)) {
        DirTree curr = (DirTree) remFromLL(bfs_list, 0);

        printTreeNode(curr, 1, 0);
        
        if (!isTreeFile(curr)) {
            /* Is a directory; add all children */
            LList children = getDirTreeChildren(curr);

            while (!isEmptyLL(children))
                appendToLL(bfs_list, remFromLL(children, 0));

            free(children);

        }

    }

    free(bfs_list);

    return 0;
    
}


int cmd_prfiles(char *argv[]) {
    DirTree root;
    LList bfs_list = makeLL();
    
    /* Get the top directory of the BFS */
    if (!argv[1])
        root = getWorkDirNode();
    else {
        char **dirtoks = str_to_vec(argv[1], '/');
        root = getRelTree(getWorkDirNode(), dirtoks);
        free_str_vec(dirtoks);
    }
    
    appendToLL(bfs_list, root);

    while (!isEmptyLL(bfs_list)) {
        DirTree curr = (DirTree) remFromLL(bfs_list, 0);
        
                        
        /* Breadth-first recursive definition */
        if (!isTreeFile(curr)) {
            /* Is a directory; add all children */
            LList children = getDirTreeChildren(curr);

            while (!isEmptyLL(children))
                appendToLL(bfs_list, remFromLL(children, 0));

            free(children);

        } else {
            /* Get block information */
            LList blocks = getTreeFileBlocks(curr);
            int num_blks = sizeOfLL(blocks);
            long *blks = (long*) malloc(num_blks * sizeof(long));
            int i;
            
            /* For printing */
            int contig = 0;
            
            /* Print basic file data */
            printTreeNode(curr, 1, 1);
            
            /* Dequeue each block number */
            for (i = 0; i < num_blks; i++)
                blks[i] = *((long*) getFromLL(blocks, 0));
            free(blocks);

            printf("%i blocks%s", num_blks, num_blks ? ": " : "");

            for (i = 0; i < num_blks-1; i++) {
                int j;
                int best = i;
                long swp;
                
                for(j = i+1; j < num_blks; j++) {
                    if (blks[j] < blks[best])
                        best = j;
                }
                
                swp = blks[i];
                blks[i] = blks[best];
                blks[best] = swp;

                if (i > 0 && blks[i] - 1 == blks[i-1]) {
                    if (!contig) {
                        printf("-");
                        contig = 1;
                    } 
                } else {
                    if (contig) {
                        printf("%ld", blks[i]);
                        contig = 0;
                    } else
                        printf(" %ld", blks[i]);
                }
            }

            printf("\n");


        }

    }

    free(bfs_list);

    return 0;

}


int cmd_prdisks(char *argv[]) {
    error_message(argv[0], "Not yet implemented."); return 0;
}




