#include "dirtree.h"
#include "cmds.h"
#include "simsys.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>


int main(int argc, char *argv[]) {
    
    /* Initialize the filesystem */
    init_filesystem(32, 1024);

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

