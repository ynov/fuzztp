#include "fuzztp_lib.h"
#ifdef BUILD_CLIENT

/* DECLARATION */
static struct fuzztp_client f;

static void init_f();
static int fuzztpc_connect(char *srv_addr);
static int fuzztpc_sendsrvmsg(const char *msg,
        int msg_size, char *res, int res_size);

static int fuzztpc_retrieve(char *path);
static int fuzztpc_store(char *path);
static int fuzztpc_quit();
static int fuzztpc_list(char *path);
static int fuzztpc_cwd(char *path);
static int fuzztpc_cd(char *path);

static void fuzztpc_parse_command(char *command, int *loop_status);
/* END DECLARATION */

/******************************************************************************/
static void init_f()
{
    f.connect_status = DISCONNECTED;
    memset(&f.hints, 0, sizeof(f.hints));
    f.hints.ai_family = AF_INET;
    f.hints.ai_socktype = SOCK_STREAM;
}

/******************************************************************************/
static int fuzztpc_connect(char *srv_addr)
{
    char res[STDBUFFSIZE];

    getaddrinfo(srv_addr,
            FUZZTPPORT,
            &f.hints,
            &f.srv_info);

    if ((f.socket_fd = socket(
            f.srv_info->ai_family,
            f.srv_info->ai_socktype,
            f.srv_info->ai_protocol)) == -1) {
        printf("| ERROR! Couldn't make socket!\n");
        close(f.socket_fd);
        return CI_ERROR;
    }

    if ((connect(f.socket_fd,
            f.srv_info->ai_addr,
            f.srv_info->ai_addrlen)) == -1) {
        printf("| ERROR! Couldn't connect to the server!\n");
        return CI_ERROR;
    }

    f.srv_ipaddr = inet_ntoa(
            (*(struct sockaddr_in*) f.srv_info->ai_addr).sin_addr);
    printf("| Connecting to %s\n", f.srv_ipaddr);

    freeaddrinfo(f.srv_info);

    recv(f.socket_fd, res, STDBUFFSIZE, 0);

    printf("|| %s\n", res);
    res[3] = '\0';
    if (strequal(res, SR200)) {
        printf("| Connected!\n");
        f.connect_status = CONNECTED;
    }

    return CI_CONN;
}

/******************************************************************************/
static int fuzztpc_sendsrvmsg(const char *msg,
        int msg_size, char *res, int res_size)
{
    int last_byte;

    if (send(f.socket_fd, msg, msg_size, 0) == -1) {
        printf("| ERROR! Send-Receive server message error!\n");
        return -1;
    }

    if ((last_byte = recv(f.socket_fd, res, res_size, 0)) == -1) {
        printf("| ERROR! Send-Receive server message error!\n");
        return -1;
    }

    res[last_byte] = '\0';
    return 0;
}

/******************************************************************************/
static int fuzztpc_retrieve(char *path)
{
    if (f.connect_status == DISCONNECTED) {
        printf("| ERROR! No open connection!\n");
        return CI_ERROR;
    }

    /* TODO */

    return CI_RETR;
}

/******************************************************************************/
static int fuzztpc_store(char *path)
{
    if (f.connect_status == DISCONNECTED) {
        printf("| ERROR! No open connection!\n");
        return CI_ERROR;
    }

    /* TODO */

    return CI_STOR;
}

/******************************************************************************/
static int fuzztpc_quit()
{
    if (f.connect_status == DISCONNECTED) {
        printf("| ERROR! No open connection!\n");
        return CI_ERROR;
    }

    char msg[] = CMD_QUIT;
    char res[MEDIUMBUFFSIZE];

    fuzztpc_sendsrvmsg(msg, sizeof(msg), res, sizeof(res));
    printf("|| %s\n", res);
    res[strlen(SR200)] = '\0';

    if (strequal(res, SR200)) {
        close(f.socket_fd);
        f.connect_status = DISCONNECTED;
        printf("| Disconnected!\n");
    }

    return CI_QUIT;
}

/******************************************************************************/
static int fuzztpc_list(char *path)
{
    char msg[STDBUFFSIZE];
    char res[BIGBUFFSIZE];
    int lb;

    memset(&msg, 0, sizeof(msg));
    if (path != NULL) {
        sprintf(msg, CMD_LIST " %s", path);
    } else {
        strcpy(msg, CMD_LIST);
    }

    fuzztpc_sendsrvmsg(msg, strlen(msg), res, SMALLBUFFSIZE);
    printf("|| %s\n", res);
    res[strlen(SR150)] = '\0';

    if (strequal(res, SR150)) {
        lb = recv(f.socket_fd, res, sizeof(res), 0);
        res[lb] = '\0';
        printf("\n%s\n", res);
    } else {
        printf("| ERROR while listing contents on the server!\n");
    }

    return CI_LIST;
}

/******************************************************************************/
static int fuzztpc_cwd(char *path)
{
    if (f.connect_status == DISCONNECTED) {
        printf("| ERROR! No open connection!\n");
        return CI_ERROR;
    }

    char msg[STDBUFFSIZE];
    char res[MEDIUMBUFFSIZE];

    memset(&msg, 0, sizeof(msg));
    sprintf(msg, CMD_CWD " %s", path);

    fuzztpc_sendsrvmsg(msg, strlen(msg), res, sizeof(res));
    printf("|| %s\n", res);
    res[strlen(SR200)] = '\0';

    if (strequal(res, SR200)) {
        printf("| Server's working directory sucessfully changed to %s.\n", path);
    } else {
        printf("| ERROR! Couldn't change server's working directory to %s.\n", path);
        printf("| Invalid path or permission denied.\n");
    }

    return CI_CWD;
}

/******************************************************************************/
static int fuzztpc_cd(char *path)
{
    if (chdir(path) == -1) {
        printf("| ERROR while changing working directory to %s\n", path);
        return CI_ERROR;
    } else {
        printf("| Successfully changed working directory to %s\n", path);
        printf("| cwd: %s\n", getcwd(NULL, STDBUFFSIZE));
    }
    return CI_CD;
}

/******************************************************************************/
static void fuzztpc_parse_command(char *command, int *loop_status)
{
    char **cmd_arr;
    int arr_len;

    arr_len = fuzztp_strtoken(command, &cmd_arr, ' ', 4);

    /* CONN */
    if (strequal(cmd_arr[0], CMD_CONN)) {
        if (arr_len == 2) {
            fuzztpc_connect(cmd_arr[1]);
        } else {
            printf("| ERROR! Use: CONN <ip_address>\n");
        }
        free(cmd_arr);
        return;
    }

    /* RETR */
    if (strequal(cmd_arr[0], CMD_RETR)) {
        if (arr_len == 2) {
            fuzztpc_retrieve(cmd_arr[1]);
        } else {
            printf("| ERROR! Use: RETR <path>\n");
        }
        free(cmd_arr);
        return;
    }

    /* STOR */
    if (strequal(cmd_arr[0], CMD_STOR)) {
        if (arr_len == 2) {
            fuzztpc_store(cmd_arr[1]);
        } else {
            printf("| ERROR! Use: PATH <path>\n");
        }
        return;
    }

    /* QUIT */
    if (strequal(cmd_arr[0], CMD_QUIT)) {
        fuzztpc_quit();
        free(cmd_arr);
        return;
    }

    /* LIST */
    if (strequal(cmd_arr[0], CMD_LIST)) {
        if (arr_len == 2) {
            fuzztpc_list(cmd_arr[1]);
        } else {
            fuzztpc_list(NULL);
        }
        free(cmd_arr);
        return;
    }

    /* CWD */
    if (strequal(cmd_arr[0], CMD_CWD)) {
        if (arr_len == 2) {
            fuzztpc_cwd(cmd_arr[1]);
        } else {
            printf("| ERROR! Use: CWD <path>\n");
        }
        free(cmd_arr);
        return;
    }

    /* CD */
    if (strequal(cmd_arr[0], CMD_CD)) {
        if (arr_len == 2) {
            fuzztpc_cd(cmd_arr[1]);
        } else {
            printf("| ERROR! Use: CD <path>\n");
        }
        free(cmd_arr);
        return;
    }

    /* SHUTDOWN */
    if (strequal(cmd_arr[0], CMD_SHUTDOWN)) {
        if (f.connect_status == CONNECTED) {
            fuzztpc_quit();
        }

        *loop_status = 0;
        free(cmd_arr);
        return;
    }

    /* UNDEFINED */
    printf("| Unknown command!\n");
    free(cmd_arr);
    return;
}

/******************************************************************************/
int fuzztp_client_main(int argc, char **argv)
{
    int loop;
    char command[INPUTBUFFSIZE];

    strcpy(f.cwd, fuzztp_getcwd(argc, argv));

    printf("fuzztp client\n");
    printf("cwd: %s\n", f.cwd);

    init_f();

    loop = 1;
    while(loop) {
        printf("\nfuzztp >> ");
        fuzztp_gets(command);
        fuzztpc_parse_command(command, &loop);
    }

    return 0;
}

#endif /* BUILD_CLIENT */
