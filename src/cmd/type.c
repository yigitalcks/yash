#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "cmd/type.h"
#include "token.h"


int is_builtin(char* arg) {
    for (int i = 0; i < num_builtins; i++) {
        if(!strcmp(arg, builtins[i]))
            return 1;
    }
    return 0;
}

int is_executable(char* arg, char target[], char* paths[], int num_paths) {

    for(int i = 0; i < num_paths; i++) {
        DIR* dirp = opendir(paths[i]);
        if (!dirp) {
            continue;
        }

        errno = 0;
        struct dirent* dp;
        while ((dp = readdir(dirp)) != NULL) {
            if(!strcmp(dp->d_name, arg)) {
                snprintf(target, MAX_PATH_LENGTH, "%s/%s", paths[i], arg);
                closedir(dirp);
                return 1; // FOUND
            } 
        }
        if (errno != 0) {
            closedir(dirp);
            return -1; // READ_ERROR
        }

        closedir(dirp);
    }
    return 0; // NOT_FOUND
}