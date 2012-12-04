#ifndef FUZZTP_LIB_H
#define FUZZTP_LIB_H

#ifndef SPLIT_BUILD
#   define BUILD_SERVER
#   define BUILD_CLIENT
#else
#   ifndef BUILD_SERVER
#       define BUILD_CLIENT
#   endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <dirent.h>

#define BIGBUFFSIZE 2048
#define STDBUFFSIZE 256
#define INPUTBUFFSIZE 256
#define MEDIUMBUFFSIZE 128
#define SMALLBUFFSIZE 32

#define BACKLOG 10

#define FUZZTPPORT "15601"
#define QUITMSG "QUIT"

#define DISCONNECTED 0
#define CONNECTED 1

#define CMD_CONN        "CONN"
#define CMD_RETR        "RETR"
#define CMD_STOR        "STOR"
#define CMD_QUIT        "QUIT"
#define CMD_LIST        "LIST"
#define CMD_CWD         "CWD"
#define CMD_CD          "CD"
#define CMD_SHUTDOWN    "SHUTDOWN"

#define CI_CONN         1
#define CI_RETR         2
#define CI_STOR         3
#define CI_QUIT         4
#define CI_LIST         5
#define CI_CWD          6
#define CI_CD           7
#define CI_SHUTDOWN     8
#define CI_UNDEFINED    9
#define CI_ERROR        10

/** BEGIN FUZZTP_CLIENT *******************************************************/

struct fuzztp_client {
    int connect_status;
    int socket_fd;
    struct addrinfo hints;
    struct addrinfo *srv_info;
    char *srv_ipaddr;
    char srv_hostname[STDBUFFSIZE];
    char cwd[STDBUFFSIZE];
};

/** END FUZZTP_CLIENT *********************************************************/

/** BEGIN FUZZTP_SERVER *******************************************************/

/* SR, Server Response */
#define SR150   "150"   /* ~(CONN, QUIT, CWD) */
#define SR250   "250"   /* Send/Reveive data */
#define SR200   "200"   /* (CONN, QUIT, CWD) */
#define SR500   "500"   /* Invalid command */
#define SR501   "501"   /* Invalid path */

struct fuzztp_server {
    int socket_fd;
    int accsocket_fd;
    struct addrinfo hints;
    struct addrinfo *srv_info;
    struct sockaddr_storage client_addr;
    struct sigaction sa;
};

/** END FUZZTP_SERVER *********************************************************/


/* FUZZTPLIB */
int strequal(const char *a, const char *b);
char *fuzztp_getcwd(int argc, char **argv);
char *fuzztp_gets(char *s);
int fuzztp_strtoken(char *str, char ***str_arr, char tok, int max_arr_len);

#endif /* FUZZTP_LIB_H */
