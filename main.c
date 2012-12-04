#include <string.h>
#include "fuzztp_lib.h"

#ifdef BUILD_SERVER
extern int fuzztp_server_main(int argc, char **argv);
#endif

#ifdef BUILD_CLIENT
extern int fuzztp_client_main(int argc, char **argv);
#endif

int main(int argc, char **argv) {
#ifndef SPLIT_BUILD
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--server")) {
            return fuzztp_server_main(argc, argv);
        }
    }

    return fuzztp_client_main(argc, argv);
#else
#   ifdef BUILD_SERVER
    return fuzztp_server_main(argc, argv);
#   else
    return fuzztp_client_main(argc, argv);
#   endif
#endif
}
