#include <string.h>

extern int fuzztp_server(int argc, char **argv);
extern int fuzztp_client(int argc, char **argv);

int main(int argc, char **argv) {
    int i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "--server")) {
            return fuzztp_server(argc, argv);
        }
    }

    return fuzztp_client(argc, argv);
}
