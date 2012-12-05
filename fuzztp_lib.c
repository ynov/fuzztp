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

int fuzztp_fexist(char *path, char *errmsg)
{
    int st_res;
    struct stat st;

    st_res = stat(path, &st);
    if (st_res == -1) {
        sprintf(errmsg, "%s doesn't exist or permission denied!", path);
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        sprintf(errmsg, "%s is a directory, not a regular file!", path);
        return -1;
    }

    return 0;
}

void fuzztp_get_filename_from_path(char *path, char *filename)
{
    int nidx;
    char **splitted_path;

    nidx = fuzztp_strtoken(path, &splitted_path, '/', 64);
    strcpy(filename, splitted_path[nidx - 1]);

    free(splitted_path);
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
