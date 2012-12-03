#include "fuzztp_lib.h"

// DECLARATION
static struct fuzztp_client f;

static void init_f();
static int fuzztpc_connect(char *srv_addr);
static int fuzztpc_sendsrvmsg(const char *msg,
        int msg_size, char *res, int res_size);
static int fuzztpc_store(char *path);
static int fuzztpc_quit();
static int fuzztpc_parse_command(char *command);
// END DECLARATION

/******************************************************************************/
static void init_f()
{
    f.connect_status = DISCONNECTED;
    f.hints.ai_family = AF_INET;
    f.hints.ai_socktype = SOCK_STREAM;
}

/******************************************************************************/
static int fuzztpc_connect(char *srv_addr)
{
    getaddrinfo(srv_addr,
            FUZZTPPORT,
            &f.hints,
            &f.srv_info);

    if ((f.socket_fd = socket(
            f.srv_info->ai_family,
            f.srv_info->ai_socktype,
            f.srv_info->ai_protocol)) == -1) {
        printf("ERROR! Couldn't make socket!\n");
        close(f.socket_fd);
        return CI_ERROR;
    }

    if ((connect(f.socket_fd,
            f.srv_info->ai_addr,
            f.srv_info->ai_addrlen)) == -1) {
        printf("ERROR! Couldn't connect to the server!\n");
        return CI_ERROR;
    }

    f.srv_ipaddr = inet_ntoa(
            (*(struct sockaddr_in*) f.srv_info->ai_addr).sin_addr);
    printf("Connecting to %s\n", f.srv_ipaddr);

    freeaddrinfo(f.srv_info);

    f.connect_status = CONNECTED;

    return CI_CONN;
}

/******************************************************************************/
static int fuzztpc_sendsrvmsg(const char *msg,
        int msg_size, char *res, int res_size)
{
    int last_byte;

    if (send(f.socket_fd, msg, msg_size, 0) == -1) {
        printf("ERROR! Send-Receive server message error!\n");
        return -1;
    }

    if ((last_byte = recv(f.socket_fd, res, res_size, 0)) == -1) {
        printf("ERROR! Send-Receive server message error!\n");
        return -1;
    }

    res[last_byte - 1] = '\0';
    return 0;
}

/******************************************************************************/
static int fuzztpc_retrieve(char *path)
{
    if (f.connect_status == DISCONNECTED) {
        printf("ERROR! No open connection!\n");
        return CI_ERROR;
    }
    return CI_RETR;
}

/******************************************************************************/
static int fuzztpc_store(char *path)
{
    if (f.connect_status == DISCONNECTED) {
        printf("ERROR! No open connection!\n");
        return CI_ERROR;
    }
    return CI_STOR;
}

/******************************************************************************/
static int fuzztpc_list(char *path)
{
    return CI_LIST;
}

/******************************************************************************/
static int fuzztpc_cwd(char *path)
{
    if (f.connect_status == DISCONNECTED) {
        printf("ERROR! No open connection!\n");
        return CI_ERROR;
    }
    return CI_CWD;
}

/******************************************************************************/
static int fuzztpc_cd(char *path)
{
    return CI_CD;
}

/******************************************************************************/
static int fuzztpc_quit()
{
    if (f.connect_status == DISCONNECTED) {
        printf("ERROR! No open connection!\n");
        return CI_ERROR;
    }

    char msg[] = QUITMSG;
    char res[SMALLBUFFSIZE];

    fuzztpc_sendsrvmsg(msg, sizeof(msg), res, sizeof(res));

    if(strequal(res, QUITMSG)) {
        close(f.socket_fd);
        f.connect_status = DISCONNECTED;
    }

    return CI_QUIT;
}

/******************************************************************************/
static int fuzztpc_parse_command(char *command)
{
    char **cmd_arr;
    int arr_len;

    arr_len = fuzztp_strtoken(command, &cmd_arr, ' ', 16);

    /* CONN */
    if (strequal(cmd_arr[0], CMD_CONN)) {
        if (arr_len == 2) {
            return fuzztpc_connect(cmd_arr[1]);
        } else {
            printf("ERROR! Use: CONN <ip_address>\n");
        }
    }

    /* RETR */
    else if (strequal(cmd_arr[0], CMD_RETR)) {
        if (arr_len == 2) {
            return fuzztpc_retrieve(cmd_arr[1]);
        } else {
            printf("ERROR! Use: RETR <path>\n");
        }
    }

    /* STOR */
    else if (strequal(cmd_arr[0], CMD_STOR)) {
        if (arr_len == 2) {
            return fuzztpc_store(cmd_arr[1]);
        } else {
            printf("ERROR! Use: PATH <path>\n");
        }
    }

    /* QUIT */
    else if (strequal(cmd_arr[0], CMD_QUIT)) {
        return fuzztpc_quit();
    }

    /* LIST */
    else if (strequal(cmd_arr[0], CMD_LIST)) {
        if (arr_len == 2) {
            fuzztpc_list(cmd_arr[1]);
        } else {
            fuzztpc_list(NULL);
        }
        return CI_LIST;
    }

    /* CWD */
    else if (strequal(cmd_arr[0], CMD_CWD)) {
        if (arr_len == 2) {
            return fuzztpc_cwd(cmd_arr[1]);
        } else {
            printf("ERROR! Use: CWD <path>\n");
        }
    }

    /* CD */
    else if (strequal(cmd_arr[0], CMD_CD)) {
        if (arr_len == 2) {
            return fuzztpc_cd(cmd_arr[1]);
        } else {
            printf("ERROR! Use: CD <path>\n");
        }
    }

    /* SHUTDOWN */
    else if (strequal(cmd_arr[0], CMD_SHUTDOWN)) {
        if (f.connect_status == CONNECTED) {
            fuzztpc_quit();
        }
        return CI_SHUTDOWN;
    }

    return CI_UNDEFINED;
}

/******************************************************************************/
int fuzztp_client_main(int argc, char **argv)
{
    char command[INPUTBUFFSIZE];
    int c_id;

    strcpy(f.cwd, fuzztp_getcwd(argc, argv));

    printf("fuzztp client\n");
    printf("cwd: %s\n", f.cwd);

    init_f();

    for(;;) {
        printf("fuzztp >> ");
        fuzztp_gets(command);
        c_id = fuzztpc_parse_command(command);

        if (c_id == CI_SHUTDOWN) {
            break;
        }
    }

    return 0;
}
