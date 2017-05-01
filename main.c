#define __USE_XOPEN
#define _GNU_SOURCE

#include "dirtree.h"
#include "cmds.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <unistd.h>

struct file_loaddata {
    char *name;
    long filesize;
    time_t timestamp;
};

void getDirsFromFile(FILE *file, LList dirlist) {
    char pathbuff[2048];

    int dirs = 0;

    while (fgets(pathbuff, 2048, file) != NULL) {
        char *path;
        
        pathbuff[strlen(pathbuff)-1] = '\0';
        path = (char*) malloc((1 + strlen(pathbuff)) * sizeof(char));
        strcpy(path, pathbuff);

        appendToLL(dirlist, path);

        dirs++;
    }

    printf("Loaded %i directories\n", dirs);
}


void getFilesFromFile(FILE *file, LList filelist) {
    
    /* Used for getting file data */
    char pathbuff[2048];
    char datebuff[32];
    long sizebuff;

    int files = 0;
    
    /* Timestamp data */
    time_t read_time = time(NULL);
    struct tm *str_tm = localtime(&read_time);
    int curr_year = str_tm->tm_year;

    while (fscanf(file, "%*s %*s %*s %*s %*s %*s") != EOF) {
        int i;
        char m_time[8]; /* Month */
        char d_time[8]; /* Day */
        char p_time[8]; /* Parameter */

        /* Read time and filename */
        fscanf(file, "%ld ", &sizebuff);
        fscanf(file, "%s ", m_time);
        fscanf(file, "%s ", d_time);
        fscanf(file, "%s ", p_time);
        fgets(pathbuff, 2048, file);

        /* Build date */
        sprintf(datebuff, "%s %s %s", m_time, d_time, p_time);
        if (!strptime(datebuff, "%b %d %H:%M", str_tm))
            strptime(datebuff, "%b %d %Y", str_tm);
        else
            str_tm->tm_year = curr_year;
        
        /* Get rid of the newline */
        for (i = 0; pathbuff[i] != '\n'; i++);
        pathbuff[i] = '\0';

        /* Make a data structure */
        struct file_loaddata *dta = (struct file_loaddata*) malloc(sizeof(struct file_loaddata));
        dta->name = (char*) malloc((strlen(pathbuff) + 1) * sizeof(char));
        strcpy(dta->name, pathbuff);
        dta->filesize = sizebuff;
        dta->timestamp = mktime(str_tm);
        
        /* Add to the list */
        appendToLL(filelist, dta);

        files++;
    }

    printf("Loaded %i files\n", files);

}

int main(int argc, char *argv[]) {
    
    /* the filesystem and block sizes, in bytes */
    long blk_size = 0;
    long fs_size = 0;
    
    /* List of all of the files and directories to add */
    LList dir_data = makeLL();
    LList file_data = makeLL();

    LLiter dir_iter;
    LLiter file_iter;

    int i; 
    
    /* Load the files */
    for (i = 1; argv[i]; i++) {
        if (!strcmp(argv[i], "-b")) {
            /* User sets a value for the block size */
            if (argv[i+1]) {
                blk_size = atol(argv[i+1]);
                i++;
            } else {
                printf("\033[1m\033[33mWarning\033[0m: Provided flag %s without value\n", argv[i]);
            }
        } else if (!strcmp(argv[i], "-s")) {
            /* User has a custom filesystem capacity */
            if (argv[i+1]) {
                fs_size = atol(argv[i+1]);
                i++;
            } else
                printf("\033[1m\033[33mWarning\033[0m: Provided flag %s without value\n", argv[i]);
        } else if (!strcmp(argv[i], "-f")) {
            /* User has files to add */
            if (argv[i+1]) {
                FILE *file = fopen(argv[i+1], "r");

                if (file) {
                    getFilesFromFile(file, file_data);
                    fclose(file);
                } else
                    printf("\033[1m\033[31mError\033[0m: File %s could not be opened\n", argv[i+1]);

                i++;
            } else
                printf("\033[1m\033[33mWarning\033[0m: Provided flag %s without value\n", argv[i]);

        } else if (!strcmp(argv[i], "-d")) {
            /* User has directories to add */
            if (argv[i+1]) {
                FILE *file = fopen(argv[i+1], "r");

                if (file) {
                    getDirsFromFile(file, dir_data);
                    fclose(file);
                } else
                    printf("\033[1m\033[31mError\033[0m: File %s could not be opened\n", argv[i+1]);

                i++;
            } else
                printf("\033[1m\033[33mWarning\033[0m: Provided flag %s without value\n", argv[i]);


        }
    }

    if (!blk_size) {
        /* Check whether or not a block size was given */
        printf("\033[1m\033[33mWarning\033[0m: Block size not specified; defaulting to 512B\n");
        blk_size = 512;
    } else {
        printf("Using block size of %ldB\n", blk_size);
    }
    
    if (!fs_size) {
        /* Check whether or not a filesystem capacity was given */
        printf("\033[1m\033[33mWarning\033[0m: Filesystem size not specified; defaulting to 64kB\n");
        fs_size = 65536;
    } else {
        printf("Using filesystem size of %ldB\n", fs_size);
    }

    /* Initialize the filesystem */
    init_filesystem(blk_size, fs_size);

    /* Put all of the directories in */
    dir_iter = makeLLiter(dir_data);
    while (iterHasNextLL(dir_iter)) {
        char *path = (char*) iterNextLL(dir_iter);
        char **pathtoks = str_to_vec(path, '/');
        int n;
        
        /* Add the directory */
        if (!pathtoks[1] || (n = addDirToTree(getRootNode(), &pathtoks[1])))
            printf("\033[1m\033[31mError\033[0m: Could not add directory '%s'; error code %i\n", path, n);


        free_str_vec(pathtoks);
    }
    
    /* Put all of the files in */
    file_iter = makeLLiter(file_data);
    while (iterHasNextLL(file_iter)) {
        struct file_loaddata *dta = iterNextLL(file_iter);
        char **pathtoks = str_to_vec(dta->name, '/');

        long blks = 1 + (dta->filesize - 1) / blk_size;
        
        /* Check to see if enough memory is present */
        if (enoughMemFor(blks)) {
            int n;
            /* Can only complete process on success */
            if ((n = addFileToTree(getRootNode(), &pathtoks[1])))
                printf("\033[1m\033[31mError\033[0m: Could not add file '%s'; error code %i\n", dta->name, n);
            else {
                DirTree node = getDirSubtree(getRootNode(), &pathtoks[1]);

                /* Acquire enough memory spaces */
                while (blks) {
                    assignMemoryBlock(node, allocBlock());
                    blks--;
                }
                updateFileSize(node, dta->filesize);
                
                              
            }
        } else {
            printf("\033[1m\033[31mError\033[0m: Insufficient memory for file '%s' (needs %ld blocks)\n", dta->name, blks);
        }
        
    }

    /* Initial condition: all directories were made at the end of '69 */
    while (!isEmptyLL(dir_data)) {
        char *path = (char*) remFromLL(dir_data, 0);
        char **pathtoks = str_to_vec(path, '/');

        DirTree dir = getDirSubtree(getRootNode(), &pathtoks[1]);
        setTimestamp(dir, 0);

        free_str_vec(pathtoks);
        free(path);

    }
    
    /* Update file timestamps */
    while (!isEmptyLL(file_data)) {
        struct file_loaddata *dta = remFromLL(file_data, 0);
        char **pathtoks = str_to_vec(dta->name, '/');

        DirTree node = getDirSubtree(getRootNode(), &pathtoks[1]);
        DirTree parent = getTreeParent(node);

        time_t timestamp = dta->timestamp;

         /* Update the timestamp */
        setTimestamp(node, timestamp);
                 
        /* Do the same for all parents */
        if (difftime(getTreeTimestamp(parent), timestamp) < 0) {
            setTimestamp(parent, timestamp);
        }

        /* Free memory blocks */
        free(pathtoks);
        free(dta->name);
        free(dta);

    }

    /* Free lists and iterators */
    disposeIterLL(dir_iter);
    disposeIterLL(file_iter);
    free(dir_data);
    free(file_data);

    while (1) {
        char buff[64];
        char *cmd = malloc(sizeof(char));
        char **arg_vec;
        int n, len;
        
        len = 0;
        
        printf("\033[1m\033[32m" "root@filesystem\033[0m:\033[1m\033[34m");
        
        /* Show path */
        arg_vec = pathVecOfTree(getWorkDirNode());
        for (n = 0; arg_vec[n]; n++)
            printf("%s%s", arg_vec[n], arg_vec[n+1] ? "/" : "");
        free_str_vec(arg_vec);

        printf("\033[0m$ ");
        fflush(stdout);

        /* Read from stdin */
        while ((n = read(0, buff, 64))) {
            char *tmp = (char*) realloc(cmd, (len+65)*sizeof(char));

            if (!tmp) {
                tmp = (char*) malloc((len+65) * sizeof(char));
                strcpy(tmp, cmd);
                free(cmd);
            }
            cmd = tmp;

            n = 0;
            while (n < 64) {
                if (buff[n] == '\n') {
                    cmd[len+n] = '\0';
                    break;
                }
                cmd[len+n] = buff[n];
                n++;
            }

            len += n;

            if (n < 64)
                break;
        }

        cmd[len] = '\0';

        /* Tokenize the command */
        arg_vec = str_to_vec(cmd, ' ');
        free(cmd);

        /* Run the command */
        cmd_exec(arg_vec);
        free_str_vec(arg_vec);

        

    }
    
}

