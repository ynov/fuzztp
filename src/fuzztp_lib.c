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

void fuzztp_read_send_file_chunked(char *path, int socket_fd)
{
    char fsize_msg[MEDIUMBUFFSIZE];
    char msg[SMALLBUFFSIZE];
    char buff[FILEBUFFSIZE];
    FILE *file;
    int fsize;
    int fptr;
    int lb;

    file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);

    sprintf(fsize_msg, "%d", fsize);
    send(socket_fd, fsize_msg, strlen(fsize_msg), 0);
    printf("-- SEND filesize: %d bytes\n", fsize);

    lb = recv(socket_fd, msg, SMALLBUFFSIZE, 0);
    msg[lb] = '\0';

    /* send file in chunked FILEBUFFSIZE filesize */
    fptr = 0;
    while (fptr <= fsize - FILEBUFFSIZE) {
        fseek(file, fptr, SEEK_SET);
        fread(buff, FILEBUFFSIZE, 1, file);
        send(socket_fd, buff, FILEBUFFSIZE, 0);
        fptr += FILEBUFFSIZE;
        /* printf("-- SEND %d bytes\n", fptr); */
    }

    fseek(file, fptr, SEEK_SET);
    fread(buff, fsize - fptr, 1, file);
    send(socket_fd, buff, fsize - fptr, 0);
    /* printf("-- SEND %d bytes\n", fsize - fptr); */

    printf("-- END!\n");
    fclose(file);
}

void fuzztp_retrieve_write_file_chunked(char *filename, int socket_fd)
{
    char fsize_msg[MEDIUMBUFFSIZE];
    char buff[FILEBUFFSIZE];
//    char msg[SMALLBUFFSIZE];
    int lb;
    FILE *file;
    int fsize;
    int fptr;

    /* get the filesize */
    lb = recv(socket_fd, fsize_msg, MEDIUMBUFFSIZE, 0);
    fsize_msg[lb] = '\0';
    sscanf(fsize_msg, "%d", &fsize);
    printf("| RECEIVE filesize: %d bytes\n", fsize);

    send(socket_fd, "NEXT", SMALLBUFFSIZE, 0);

    /* receive chunked file, and write */
    file = fopen(filename, "wb+");

    fptr = 0;
    while (fptr <= fsize - FILEBUFFSIZE) {
        recv(socket_fd, buff, FILEBUFFSIZE, 0);
        fwrite(buff, FILEBUFFSIZE, 1, file);
        fptr += FILEBUFFSIZE;
        /* printf("| RECEIVE %d bytes\n", fptr); */
    }

    recv(socket_fd, buff, fsize - fptr, 0);
    fwrite(buff, fsize - fptr, 1, file);
    /* printf("| RECEIVE %d bytes\n", fsize - fptr); */

    fclose(file);
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
