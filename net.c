#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include "log.h"
#include "net.h"

struct Conn* init_rawconn(char *hostname, uint16_t port)
{
    int ec;
    struct addrinfo hints;

    struct Conn *conn = calloc(1, sizeof(struct Conn));
    assert(conn != NULL);

    conn->bcur = 0;
    conn->hostname = hostname;
    conn->port = port;
    conn->ssl = false;
    conn->send = raw_send;
    conn->recv = raw_recv;
    conn->close = raw_close;
    memset(&hints, 0, sizeof(struct addrinfo));
    {
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0; /* None */
        hints.ai_protocol = 0; /* Any */
    }
    ec = getaddrinfo(conn->hostname, NULL, &hints, &(conn->ai));

    if (ec != 0)
    {
        wlogf(NETERROR, "Failed to resolve hostname '%s': %s\n",
              conn->hostname, gai_strerror(ec));
        return NULL;
    }

    struct addrinfo *ai;
    for (ai = conn->ai; ai != NULL; ai = ai->ai_next)
    {
        struct sockaddr *addr = ai->ai_addr;
        void *ia;
        socklen_t sz;
        if (addr->sa_family == AF_INET)
        {
            ia = &(((struct sockaddr_in*)addr)->sin_addr);
            ((struct sockaddr_in*)addr)->sin_port = htons(conn->port);
            sz = INET_ADDRSTRLEN;
        }
        else
        {
            ia = &(((struct sockaddr_in6*)addr)->sin6_addr);
            ((struct sockaddr_in6*)addr)->sin6_port = htons(conn->port);
            sz = INET6_ADDRSTRLEN;
        }
        char ipstr[sz];
        inet_ntop(addr->sa_family, ia, ipstr, sz);
        wlogf(NETDEBUG, "Trying %s\n", ipstr);

        conn->sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (conn->sock == -1)
        {
            wlogf(NETWARN, "Socket error: %s\n", strerror(errno));
            close(conn->sock);
            continue;
        }
        if (connect(conn->sock, ai->ai_addr, ai->ai_addrlen) == 0)
        {
            wlogf(INFO, "Connected!\n");
            break;
        }
        wlogf(NETWARN, "Connect error: %s\n", strerror(errno));
    }
    if (ai == NULL)
    {
        wlogf(NETERROR, "Failed to connect to '%s'\n", conn->hostname);
        return NULL;
    }

    conn->pfd = calloc(1, sizeof(struct pollfd));
    conn->pfd->fd = conn->sock;
    conn->pfd->events = POLLIN | POLLPRI;
    conn->pfd->revents = 0;

    freeaddrinfo(conn->ai);

    return conn;
}

bool raw_send(struct Conn *this, char *bytes)
{
    int sz = strlen(bytes);

    wlogf(NETDEBUG, "Sending %d bytes: %s\n", sz, bytes);
    sz = send(this->sock, bytes, sz, 0);
    return sz < 0 ? false : true;
}

bool raw_recv(struct Conn *this)
{
    int sz, ret;
    char *bufptr = this->buffer + this->bcur;

    do
    {
        ret = poll(this->pfd, 1, 500);
        if (ret == -1) wlogf(NETERROR, "Poll error: %s\n", strerror(errno));
        if (ret <= 0) break;
        else if (this->pfd->revents & (POLLERR | POLLHUP | POLLNVAL)) return false;
        else if (this->pfd->revents & POLLIN)
        {
            sz = recv(this->sock, bufptr, MAXBUF - this->bcur, 0);
            if (sz < 0) return false;
            wlogf(NETTRACE, "Recieved %d bytes (@%d): %s\n", sz, this->bcur, bufptr);
            bufptr += sz;
            this->bcur += sz;
        }
    } while (sz > 0);
    wlogf(NETDEBUG, "Read %d bytes: %s\n", this->bcur, this->buffer);
    return true;
}

void raw_close(struct Conn *this)
{
    free(this->pfd);
}

struct Conn* init_sslconn(char *hostname, uint16_t port)
{
    struct Conn *conn = init_rawconn(hostname, port);

    conn->ssl = true;
    conn->send = ssl_send;
    conn->recv = ssl_recv;
    conn->close = ssl_close;
    return conn;
}

bool ssl_send(struct Conn *this, char *bytes)
{
    wlogf(CATASTROPHE, "ssl_send: Not implemented\n");
    return false;
}

bool ssl_recv(struct Conn *this)
{
    wlogf(CATASTROPHE, "ssl_recv: Not implemented\n");
    return false;
}

void ssl_close(struct Conn *this)
{
    wlogf(CATASTROPHE, "ssl_close: Not implemented\n");
}
