#include "fuzztp_lib.h"

int fuzztp_client(int argc, char **argv)
{
    char *cwd;

    cwd = fuzztp_getcwd(argc, argv);

    printf("fuzztp client\n");
    printf("cwd: %s\n", cwd);

    return 0;
}
