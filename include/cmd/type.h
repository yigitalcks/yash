#ifndef TYPE_H
#define TYPE_H

#define MAX_PATH_LENGTH 256

int is_builtin(char* arg);
int is_executable(char* arg, char target[], char* paths[], int num_paths);

#endif