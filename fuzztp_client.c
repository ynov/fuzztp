#include "fuzztp_lib.h"

int fuzztp_client_main(int argc, char **argv)
{
    char *cwd;
    char command[INPUTBUFFSIZE];

    cwd = fuzztp_getcwd(argc, argv);

    printf("fuzztp client\n");
    printf("cwd: %s\n", cwd);

    for(;;) {
        printf("fuzztp >> ");
        fuzztp_gets(command);
        printf("command: (%s)\n", command);
        if (strequal(command, "QUIT")) {
            break;
        }
    }

    return 0;
}
