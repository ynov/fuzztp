#include "fuzztp_lib.h"
#ifdef BUILD_SERVER

static struct fuzztp_server f;
static int conn_id;

static void init_f();
static void fuzztps_listen();
static void fuzztps_accept();
static void fuzztps_handle_conn(const char *client_addr);

static int fuzztps_retrieve(char *path);
static int fuzztps_store(char *filename);
static int fuzztps_quit(const char *client_addr);
static int fuzztps_list(char *path);
static int fuzztps_cwd(char *path);

static void fuzztps_parse_msg(char *client_msg, const char *client_addr, int *loop_status);

/** ------------------------------------------------------------------------ **/

static void init_f()
{
    memset(&f.hints, 0, sizeof(f.hints));
    f.hints.ai_family = AF_INET;
    f.hints.ai_socktype = SOCK_STREAM;
    f.hints.ai_flags = AI_PASSIVE;
}

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

static void sigchld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

static void fuzztps_accept()
{
    char *client_addr_str;
    unsigned int client_addrlen;

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
            fuzztps_handle_conn(client_addr_str);

            close(f.accsocket_fd);
            printf("Connection to %s closed. (CONN_ID: %d)\n", client_addr_str, conn_id);
            exit(0);
        }

        close(f.accsocket_fd);
    }
}

static void fuzztps_handle_conn(const char *client_addr)
{
    char client_msg[STDBUFFSIZE];
    char msg[STDBUFFSIZE];
    int loop;
    int lb; /* last byte */

    sprintf(msg, SR200 " [Server connected to %s]", client_addr);
    send(f.accsocket_fd, msg, strlen(msg), 0);

    loop = 1;
    while (loop) {
        lb = recv(f.accsocket_fd, client_msg, STDBUFFSIZE, 0);
        client_msg[lb] = '\0';

        fuzztps_parse_msg(client_msg, client_addr, &loop);
    }
}

static int fuzztps_retrieve(char *path)
{
    char errmsg[MEDIUMBUFFSIZE];
    char msg[STDBUFFSIZE];

    printf("RETR: %s (CONN_ID: %d)\n", path, conn_id);

    if (fuzztp_fexist(path, errmsg) == -1) {
        printf("-- FAIL, %s\n", errmsg);
        sprintf(msg, SR501 " [%s]", errmsg);
        send(f.accsocket_fd, msg, strlen(msg), 0);

        return CI_ERROR;
    }

    strcpy(msg, SR150);
    send(f.accsocket_fd, msg, strlen(msg), 0);

    printf("-- Begin file transfer...\n");
    fuzztp_read_send_file_chunked(path, f.accsocket_fd);
    printf("-- End file transfer.\n");

    return CI_RETR;
}

static int fuzztps_store(char *filename)
{
    char msg[STDBUFFSIZE];

    printf("STOR: %s (CONN_ID: %d)\n", filename, conn_id);

    strcpy(msg, SR150);
    send(f.accsocket_fd, msg, strlen(msg), 0);

    return CI_STOR;
}

static int fuzztps_quit(const char *client_addr)
{
    char msg[STDBUFFSIZE];

    sprintf(msg, SR200 " [Server closed the connection to %s]", client_addr);
    send(f.accsocket_fd, msg, strlen(msg), 0);

    return CI_QUIT;
}

static int fuzztps_list(char *path)
{
    struct dirent **list;
    char msg[BIGBUFFSIZE];
    int i, n;

    if (path == NULL) {
        path = getcwd(NULL, STDBUFFSIZE);
    }

    printf("LIST: %s (CONN_ID: %d)\n", path, conn_id);

    n = scandir(path, &list, NULL, alphasort);

    if (n < 0) {
        strcpy(msg, SR501);
        send(f.accsocket_fd, msg, strlen(msg), 0);
        printf("-- FAILED\n");
        return CI_ERROR;
    } else {
        strcpy(msg, SR150);
        send(f.accsocket_fd, msg, strlen(msg), 0);
        printf("-- SUCCESS\n");
    }

    sprintf(msg, "CWD: %s", getcwd(NULL, STDBUFFSIZE));
    sprintf(msg, "%s\nPATH: %s", msg, path);
    sprintf(msg, "%s\n", msg);
    for (i = 0; i < n; i++) {
        if (list[i]->d_type == DT_DIR) {
            sprintf(msg, "%s\n%s/ (dir)", msg, list[i]->d_name);
        } else {
            sprintf(msg, "%s\n%s", msg, list[i]->d_name);
        }
        free(list[i]);
    }
    free(list);

    send(f.accsocket_fd, msg, strlen(msg), 0);

    return CI_LIST;
}

static int fuzztps_cwd(char *path)
{
    char msg[STDBUFFSIZE];
    printf("CWD: %s (CONN_ID: %d)\n", path, conn_id);

    if (chdir(path) == -1) {
        strcpy(msg, SR501);
        printf("-- FAILED\n");
    } else {
        strcpy(msg, SR200);
        printf("-- SUCCESS\n");
    }

    send(f.accsocket_fd, msg, strlen(msg), 0);
    return CI_CWD;
}

static void fuzztps_parse_msg(char *client_msg, const char *client_addr, int *loop_status)
{
    char **msg_arr;

    fuzztp_strtoken(client_msg, &msg_arr, ' ', 16);

    /* RETR */
    if (strequal(msg_arr[0], CMD_RETR)) {
        fuzztps_retrieve(msg_arr[1]);

        free(msg_arr);
        return;
    }

    /* STOR */
    if (strequal(msg_arr[0], CMD_STOR)) {
        fuzztps_store(msg_arr[1]);

        free(msg_arr);
        return;
    }

    /* QUIT */
    if (strequal(msg_arr[0], CMD_QUIT)) {
        fuzztps_quit(client_addr);

        *loop_status = 0;
        free(msg_arr);
        return;
    }

    /* LIST */
    if (strequal(msg_arr[0], CMD_LIST)) {
        fuzztps_list(msg_arr[1]);

        free(msg_arr);
        return;
    }

    /* CWD */
    if (strequal(msg_arr[0], CMD_CWD)) {
        fuzztps_cwd(msg_arr[1]);

        free(msg_arr);
        return;
    }

    free(msg_arr);
    return;
}

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

#endif /* BUILD_SERVER */
