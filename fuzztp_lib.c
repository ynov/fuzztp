#include "fuzztp_lib.h"

int strequal(const char *a, const char *b)
{
    return !strcmp(a, b);
}

char *fuzztp_getcwd(int argc, char **argv)
{
    char cwd[STDBUFFSIZE];
    char *ret_val;
    int i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-d")) {
            if (chdir(argv[i + 1]) == -1) {
                fprintf(stderr, "Error while changing working directory!\n");
                exit(1);
            }
            break;
        }
    }

    getcwd(cwd, sizeof(cwd));

    if (cwd[strlen(cwd) - 1] != '/') {
        strncat(cwd, "/", 1);
    }

    ret_val = (char*) calloc(strlen(cwd), sizeof(char));
    strcpy(ret_val, cwd);
    return ret_val;
}

char *fuzztp_gets(char *s)
{
    s = fgets(s, INPUTBUFFSIZE, stdin);
    s[strlen(s) - 1] = '\0';

    return s;
}

int fuzztp_strtoken(char *str, char ***str_arr, char tok, int max_arr_len)
{
    int i, j, k, len;

    *str_arr = (char**) calloc(max_arr_len, sizeof(char*));

    len = strlen(str);
    for (i = 0; i < len; i++) {
        str[i] = (str[i] == tok) ? '\0' : str[i];
    }

    j = 0;
    k = 1;
    for (i = 0; i < len; i++) {
        if (str[i] != '\0' && k == 1) {
            (*str_arr)[j++] = &str[i];
            k = 0;
        }

        else if (str[i] == '\0') {
            k = 1;
        }
    }

    return j;
}
