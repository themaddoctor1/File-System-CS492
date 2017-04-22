#include "cmds.h"
#include "dirtree.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** dir_to_vec(char *str) {
    /* First, I count the number of strings */
    int tokens = 0;
    int i, j;
    char **vec;
    
    i = 0;
    while (str[i]) {
        while (str[i] && str[i] != '/') i++;
        tokens++;
        i++;
    }
    
    vec = (char**) malloc((tokens+1) * sizeof(char*));

    i = 0;
    j = 0;
    while (str[i]) {
        int k = i;
        while (str[k] && str[k] != '/') k++;

        vec[j] = (char*) malloc((k+1-i) * sizeof(char));
        strncpy(vec[j], &str[i], k-i);
        vec[j][k] = '\0';
        
        i = k+1;
        j++;
    }

    vec[j] = NULL;

    return vec;
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

int cmd_cd(char *argv[]) {
    char **dirtoks;
    
    if (!argv[1]) {
        error_message("cd", "No arguments provided.");
        return 1;
    } else if (argv[2]) {
        error_message("cd", "Too many arguments provided.");
        return 1;
    } else {
        DirTree tgt;

        /* Build a token vector */
        dirtoks = dir_to_vec(argv[1]);

        /* Perform the function call */
        tgt = getDirSubtree(getWorkDirNode(), dirtoks);

        /* Free the vector */
        free_str_vec(dirtoks);
        
        if (!tgt) {
            error_message("cd", "Directory not found.\n");
            return 1;
        } else if (isTreeFile(tgt)) {
            error_message("cd", "Target is not a directory.\n");
            return 1;
        } else {
            setWorkDirNode(tgt);
            return 0;
        }
    }
}

int cmd_ls(char *argv[]) {
    DirTree tgt;
    DirTree working_dir = getWorkDirNode();

    if (argv[1]) {
        char **dirtoks = dir_to_vec(argv[1]);

        /* Adjust the node to print */
        tgt = getDirSubtree(working_dir, dirtoks);
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
        
        /* Go through each file, removing from the list as it goes */
        while (!isEmptyLL(files)) {
            DirTree file = (DirTree) remFromLL(files, 0);

            printf("%s", getTreeFilename(file));
            
            /* Prints only if the child is a file */
            if (isTreeFile(file))
                printf(" - %ldB",treeFileSize(file, NULL));

            printf("\n");
        }
        
        /* Delete the list */
        free(files);
    }
    
    return 0;
}

int cmd_mkdir(char *argv[]) {
    
    if (!argv[1]) {
        error_message("mkdir", "No directory names provided.");
        return 1;
    } else {
        /* Get a list of all of the files in the directory */
        LList files = getDirTreeChildren(getWorkDirNode());
        int errCode = 0;

        int i = 1;
        while (argv[i]) {
            
            int j;
            for (j = sizeOfLL(files) - 1; j >= 0; j--) {
                DirTree file = (DirTree) getFromLL(files, j);

                if (!strcmp(getTreeFilename(file), argv[i])) {
                    /* Don't make duplicate directories */
                    printf("mkdir: Cannot make directory %s: Already exists.\n", argv[i]);
                    errCode = 1;
                    break;
                }
            }

            if (j < 0) {
                /* Build a path */
                char *path[2];
                path[0] = argv[i];
                path[1] = NULL;
                
                /* Add the directory */
                addDirToTree(getWorkDirNode(), path);
            }

            i++;
        }

        while (!isEmptyLL(files))
            remFromLL(files, 0);
        free(files);
        
        return errCode;
    }

}

