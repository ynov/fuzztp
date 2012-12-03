#include <string.h>

extern int fuzztp_server_main(int argc, char **argv);
extern int fuzztp_client_main(int argc, char **argv);

int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--server")) {
            return fuzztp_server_main(argc, argv);
        }
    }

    return fuzztp_client_main(argc, argv);
}
