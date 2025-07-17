#include "token.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char* builtins[] = {
    "cd", "pwd", "echo", "exit", "type"
};
const int num_builtins = ARRAY_LEN(builtins);

const char* special_chars[] = {
    "\"", "'", "$", "`", "\\", "~"
};
const int num_special_chars = ARRAY_LEN(special_chars);

int count_char(const char* str, char target) {
    int count = 0;
    while (*str != '\0') {
        if (*str == target) {
            count++;
        }
        str++;
    }
    return count;
}

int tokenize(char* input, char* argv[]) {
    int argc = 0;
    char* p = input;

    while (*p != '\0') {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') {
            break;
        }

        argv[argc++] = p;

        int is_sq = 0;
        while (*p != '\0') {
            if(*p == '\'') {
                is_sq = !is_sq;
                memmove(p, p + 1, strlen(p + 1) + 1);
                continue;
            }

            //TODO Expanding
            switch (*p)
            {
                case '$':
                    break;
            
                default:
                    break;
            }
            
            if(!is_sq && isspace((unsigned char)*p)) {
                break;
            }
            p++;
        }

        if (*p != '\0') {
            *p = '\0';
            p++;
        }
    }

    return argc;
}
