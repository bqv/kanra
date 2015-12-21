#ifndef _NET_H_
#define _NET_H_

#include <stdbool.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include "conf.h"

#define MAXBUF 4096

struct Conn
{
    int sock, bcur;
    char buffer[MAXBUF];
    char *hostname;
    uint16_t port;
    bool ssl;

    struct addrinfo *ai;
    struct pollfd *pfd;

    void (*send)(struct Conn *this, char *bytes);
    void (*recv)(struct Conn *this);
};

struct Conn* init_rawconn(char *hostname, uint16_t port);
void raw_send(struct Conn *this, char *bytes);
void raw_recv(struct Conn *this);
struct Conn* init_sslconn(char *hostname, uint16_t port);
void ssl_send(struct Conn *this, char *bytes);
void ssl_recv(struct Conn *this);

#endif /*NET_H*/
