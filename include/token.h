#ifndef TOKEN_H
#define TOKEN_H

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

extern const char* builtins[];
extern const int num_builtins;

extern const char* special_chars[];
extern const int num_special_chars;

int tokenize(char* input, char* argv[]);
int count_char(const char* str, char target);
//extern void handle_input(char* input);

#endif