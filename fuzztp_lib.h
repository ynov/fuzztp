#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INPUTBUFFSIZE 256

char *fuzztp_getcwd(int argc, char **argv);

/* STDIN/STDOUT IO */
char *fuzztp_gets(char *s);
