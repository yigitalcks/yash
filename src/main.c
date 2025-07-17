#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <pwd.h>
#include <ctype.h>
#include <dirent.h>

#include "token.h"
#include "misc.h"

#define MAX_NUMBER_OF_PATH 32
#define MAX_PATH_LENGTH 256

#define MAX_NUM_ARGS 32
#define MAX_INPUT_SIZE 1024

enum Command {
    CMD_CD,
    CMD_PWD,
    CMD_ECHO,
    CMD_EXIT,
    CMD_TYPE,
    
    CMD_EXECUTABLE
};

enum Command parse_command(const char* command) {
    if(!strcmp(command, "cd")) return CMD_CD;  
    if(!strcmp(command, "pwd")) return CMD_PWD;
    if(!strcmp(command, "type")) return CMD_TYPE;
    if(!strcmp(command, "echo")) return CMD_ECHO;
    if(!strcmp(command, "exit")) return CMD_EXIT;

    return CMD_EXECUTABLE;
}

int is_builtin(char* arg) {
    for (int i = 0; i < num_builtins; i++) {
        if(!strcmp(arg, builtins[i]))
            return 1;
    }
    return 0;
}
// Search 
int is_executable(char* arg, char full_path[], char* paths[], int num_paths) {

    for(int i = 0; i < num_paths; i++) {
        DIR* dirp = opendir(paths[i]);
        if (!dirp) {
            continue;
        }

        errno = 0;
        struct dirent* dp;
        while ((dp = readdir(dirp)) != NULL) {
            if(!strcmp(dp->d_name, arg)) {
                snprintf(full_path, MAX_PATH_LENGTH, "%s/%s", paths[i], arg);
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

void execute_cmd(char** args) {
    int pipefd[2];
    char buf;
    if (pipe(pipefd) == -1)
        err(EXIT_FAILURE, "pipe");

    pid_t pid = fork();
    if (pid == -1) {
        err(EXIT_FAILURE, "fork");
    }

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
                
        if(strchr(args[0], '/') != NULL)
            execv(args[0], args);
        else 
            execvp(args[0], args);
            
        if(errno == ENOENT) {
            fprintf(stderr, "%s: command not found\n", args[0]);
            exit(EXIT_FAILURE);
        }
        err(EXIT_FAILURE, "exec");
    }
    else {
        // Read from parent
        if (close(pipefd[1]) == -1)  /* Close unused write end */
            err(EXIT_FAILURE, "close");
        while (read(pipefd[0], &buf, 1) > 0) {
            if (write(STDOUT_FILENO, &buf, 1) != 1)
                err(EXIT_FAILURE, "write");
        }
    
        if (close(pipefd[0]) == -1)
            err(EXIT_FAILURE, "close");
        if (wait(NULL) == -1)        /* Wait for child */
            err(EXIT_FAILURE, "wait");
    }
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);   

    char* orgPath;
    if ((orgPath = getenv("PATH")) == NULL)
        fprintf( stderr, "No PATH variable set.\n") ;
        
    char* orgPath_copy = strdup(orgPath);
    if (orgPath_copy == NULL) {
        err(EXIT_FAILURE, "orgPath strdup");
    }

    char* paths[MAX_NUMBER_OF_PATH];
    char* path = strtok(orgPath_copy, ":");

    int i = 0;
    while(path != NULL && i < 32) {
        //printf("%s\n", path);
        paths[i] = path;
        path = strtok(NULL, ":");
        i++;
    }
    int num_paths = i;
    

    while(1) {
        struct passwd *pw = getpwuid(geteuid());
        if(pw == NULL) {
            err(EXIT_FAILURE, "getpwuid");
        }

        char hostname[64];
        char current_dir[64];
        gethostname(hostname, 64);
        getcwd(current_dir, 64);

        //printf("%s%s@%s%s:%s%s%s$ ", 
        //    BLUE, 
        //    pw->pw_name, hostname, // User@Hostname
        //    RESET,
        //    GREEN,                 
        //    current_dir,           // Workingdir
        //    RESET);
        printf("$ ");

        char input[MAX_INPUT_SIZE];
        if(fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            return 1;
        }
        if(strlen(input) <= 1)
            continue;

        input[strlen(input) - 1] = '\0';

        int quote_count = count_char(input, '\'');
        char* input_p = input + strlen(input);
        while(quote_count % 2 == 1 && strlen(input) < MAX_INPUT_SIZE - 3) {
            *(input_p - 1) = '\n';
            printf("> ");

            if(fgets(input_p, MAX_INPUT_SIZE - strlen(input), stdin) == NULL) {
                perror("fgets");
                return 1;
            }
            if(strlen(input_p) <= 1) {
                continue;
            }
            input_p[strlen(input_p) - 1] = '\0';

            quote_count += count_char(input_p, '\'');
            input_p = input_p + strlen(input_p);
        }
        

        char* cargv[MAX_NUM_ARGS];
        int cargc = tokenize(input, cargv);

        enum Command cmd = parse_command(cargv[0]);
        switch (cmd) 
        {
            case CMD_CD:
                char *target_dir = NULL;

                if(cargc > 2) {
                    fprintf(stderr, "Usage: cd [dir]\n");
                    break;
                }

                if(cargc == 1) {
                    target_dir = getenv("HOME");
                    if(target_dir == NULL) {
                        target_dir = pw->pw_dir;
                    }
                } else if(cargc == 2) {
                    if(strcmp("~", cargv[1]) == 0) {
                        target_dir = getenv("HOME");
                        if(target_dir == NULL) {
                            target_dir = pw->pw_dir;
                        }
                    } else {
                        target_dir = cargv[1];
                    }        
                }
                
                if(chdir(target_dir) != 0) {
                    fprintf(stderr, "cd: %s: ", cargv[1]);
                    perror("");
                }
                break;
            case CMD_PWD:
                printf("%s\n", current_dir);
                break;

            case CMD_TYPE:
                if(is_builtin(cargv[1])) {
                    printf("%s is a shell builtin\n", cargv[1]);
                    break;
                }

                char full_path[MAX_PATH_LENGTH];
                int ret = is_executable(cargv[1], full_path, paths, num_paths);
                if (ret == 1) {
                    printf("%s is %s\n", cargv[1], full_path);
                }
                else if(ret == 0) {
                    fprintf(stderr, "%s: not found\n", cargv[1]);
                }
                else {
                    fprintf(stderr, "READ_ERROR\n");
                }

                break;
            
            case CMD_ECHO:

                for (int i = 1; i < cargc; i++) {
                    printf("%s ", cargv[i]);
                }
                printf("\n");
                break;
            
            case CMD_EXIT:
                if (cargc > 2)
                {
                    fprintf(stderr, "yash: exit: too many arguments\n");
                    break;
                }
                else if (cargc == 2)
                {
                    exit(strtol(cargv[1], NULL, 10));
                }
                exit(EXIT_SUCCESS);
            
            case CMD_EXECUTABLE:        
                cargv[cargc] = NULL;
                execute_cmd(cargv);
                break;

            default:
                fprintf(stderr, "%s: command not found\n", cargv[0]);
                break;
        }
    }
    
    return 0;
}
// Double Quotes
// Wrapping error messages with macros
