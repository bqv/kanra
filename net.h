#ifndef _NET_H_
#define _NET_H_

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include "conf.h"

#define MAXBUF 4096

typedef void (*send_fn_t)(struct Conn *this, char *bytes);
typedef void (*recv_fn_t)(struct Conn *this);

struct Conn
{
    int sock;
    size_t bcur;
    char buffer[MAXBUF];
    char *hostname;
    uint16_t port;
    bool ssl;

    struct addrinfo *ai;
    struct pollfd *pfd;

    send_fn_t send;
    recv_fn_t recv;
};

struct Conn* init_rawconn(char *hostname, uint16_t port);
void raw_send(struct Conn *this, char *bytes);
void raw_recv(struct Conn *this);
struct Conn* init_sslconn(char *hostname, uint16_t port);
void ssl_send(struct Conn *this, char *bytes);
void ssl_recv(struct Conn *this);

#endif /*NET_H*/
