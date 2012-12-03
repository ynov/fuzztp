#include "fuzztp_lib.h"

int strequal(const char *a, const char *b)
{
    return !strcmp(a, b);
}

char *fuzztp_getcwd(int argc, char **argv)
{
    char cwd[512];
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
