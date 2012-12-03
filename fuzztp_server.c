#include "fuzztp_lib.h"

int fuzztp_server(int argc, char **argv)
{
    char *cwd;

    cwd = fuzztp_getcwd(argc, argv);

    printf("fuzztp server\n");
    printf("cwd: %s\n", cwd);

    return 0;
}
