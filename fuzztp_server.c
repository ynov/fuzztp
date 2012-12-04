#include "fuzztp_lib.h"

// DECLARATION
static struct fuzztp_server f;

static void init_f();
static void fuzztps_listen();
static void fuzztps_accept();
static void fuzztps_handle_conn(int accsocket_fd, const char *client_addr);
// END DECLARATION

/******************************************************************************/
static void init_f()
{
    memset(&f.hints, 0, sizeof(f.hints));
    f.hints.ai_family = AF_INET;
    f.hints.ai_socktype = SOCK_STREAM;
    f.hints.ai_flags = AI_PASSIVE;
}

/******************************************************************************/
static void fuzztps_listen()
{
    int y = 1;
    char *srv_addr;

    getaddrinfo(NULL, FUZZTPPORT, &f.hints, &f.srv_info);

    f.socket_fd = socket(
            f.srv_info->ai_family,
            f.srv_info->ai_socktype,
            f.srv_info->ai_protocol);

    setsockopt(f.socket_fd, SOL_SOCKET, SO_REUSEADDR, &y, f.srv_info->ai_addrlen);

    bind(f.socket_fd, f.srv_info->ai_addr, f.srv_info->ai_addrlen);
    listen(f.socket_fd, BACKLOG);

    srv_addr = inet_ntoa((*(struct sockaddr_in*) f.srv_info->ai_addr).sin_addr);
    printf("Server started! Host: %s\n", srv_addr);
}

/******************************************************************************/
static void sigchld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

/******************************************************************************/
static void fuzztps_accept()
{
    char *client_addr_str;
    unsigned int client_addrlen;
    int conn_id;

    f.sa.sa_handler = sigchld_handler;
    sigemptyset(&f.sa.sa_mask);
    f.sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &f.sa, NULL);

    conn_id = 0;
    for (;;) {
        client_addrlen = sizeof(f.client_addr);
        f.accsocket_fd = accept(
                f.socket_fd,
                (struct sockaddr*) &f.client_addr,
                &client_addrlen);

        client_addr_str = inet_ntoa(
                ((struct sockaddr_in*) &f.client_addr)->sin_addr);

        conn_id++;
        printf("Got connection from %s. (CONN_ID: %d)\n", client_addr_str, conn_id);

        /* Spawn new process to handle the connection */
        if (!fork()) {
            close(f.socket_fd);

            /* Handle the connection! */
            fuzztps_handle_conn(f.accsocket_fd, client_addr_str);

            close(f.accsocket_fd);
            printf("Connection to %s closed. (CONN_ID: %d)\n", client_addr_str, conn_id);
            exit(0);
        }

        close(f.accsocket_fd);
    }
}

/******************************************************************************/
static void fuzztps_handle_conn(int accsocket_fd, const char *client_addr)
{
    char client_msg[STDBUFFSIZE];
    char msg[STDBUFFSIZE];

    sprintf(msg, "%s [Server connected to %s]", SR200, client_addr);
    send(accsocket_fd, msg, strlen(msg), 0);

    for (;;) {
        recv(accsocket_fd, client_msg, STDBUFFSIZE, 0);

        if (strequal(client_msg, QUITMSG)) {
            strcpy(msg, QUITMSG);
            send(accsocket_fd, msg, strlen(msg), 0);

            break;
        }
    }
}

/******************************************************************************/
int fuzztp_server_main(int argc, char **argv)
{
    char *cwd;
    cwd = fuzztp_getcwd(argc, argv);

    printf("fuzztp server\n");
    printf("cwd: %s\n", cwd);

    init_f();
    fuzztps_listen();
    fuzztps_accept();

    return 0;
}
