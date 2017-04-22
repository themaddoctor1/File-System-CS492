#ifndef _CMDS_H_
#define _CMDS_H_

typedef int (*SimCmd)(char**);

char** str_to_vec(char*, char);
void free_str_vec(char**);

/**
 * Executes a command in the file system.
 */
int cmd_exec(char *argv[]);

/**
 * Modifies the working directory.
 */
int cmd_cd(char *argv[]);

int cmd_ls(char *argv[]);

/**
 * File/directory creators.
 */
int cmd_mkdir(char *argv[]);
int cmd_create(char *argv[]); /* TODO: Implement */

/**
 * Modifying the size of a file.
 */
int cmd_append(char *argv[]); /* TODO: Implement */
int cmd_remove(char *argv[]); /* TODO: Implement */

int cmd_delete(char *argv[]); /* TODO: Implement */

/**
 * Terminate program.
 */
int cmd_exit(char *argv[]);   /* TODO: Implement */

/**
 * Breadth-first print of file structure.
 */
int cmd_dir(char *argv[]);    /* TODO: Implement */

/**
 * Print file/disk info.
 */
int cmd_prfiles(char *argv[]); /* TODO: Implement */
int cmd_prdisks(char *argv[]); /* TODO: Implement */

#endif
