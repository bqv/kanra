#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "util.h"
#include "log.h"
#include "irc.h"

struct IrcServer* new_server(struct Config *conf)
{
    struct IrcServer *server = malloc(sizeof(struct IrcServer));

    server->conf = conf;

    server->connect = irc_connect;
    server->read = irc_read;
    server->write = irc_write;
    return server;
}

void irc_connect(struct IrcServer *server)
{
    struct Config *conf = server->conf;
    if (conf->ssl)
    {
        server->conn = init_sslconn(conf->hostname, conf->port);
    }
    else
    {
        server->conn = init_rawconn(conf->hostname, conf->port);
    }

    irc_handshake(server);
}

void irc_handshake(struct IrcServer *this)
{
    this->conn->send(this->conn, "NICK kanra\r\n");
    this->conn->send(this->conn, "USER kanra * * me\r\n");
    struct IrcMessage *imsg = this->read(this);
    wlogf(INFO, "==Got:\n%s==", irc_mtos(imsg));
}

void irc_write(struct IrcServer *this, struct IrcMessage *message)
{
    struct Conn *conn = this->conn;
    conn->send(conn, irc_mtos(message));
}

struct IrcMessage* irc_read(struct IrcServer *this)
{
    struct IrcMessage *imsg;
    struct MessageQueue *mq = new_msgqueue();
    struct Conn *conn = this->conn;

    conn->recv(conn);
    while ((imsg = irc_stom(conn->buffer)))
    {
        if (imsg != NULL)
        {
            this->conn->bcur -= imsg->len;
            memmove(this->conn->buffer, this->conn->buffer+imsg->len, this->conn->bcur);
        }
        mq->push(mq, imsg);
    }
    return imsg;
}

struct IrcMessage* irc_stom(char *str)
{
    char *ptr, *end;

    struct IrcMessage *imsg = malloc(sizeof(struct IrcMessage));

    end = memccpy(imsg->_raw, str, '\n', MAX_IRC);
    if (end == NULL) return NULL;

    *end = '\0';
    ptr = imsg->_raw;
    imsg->len = end - ptr;
    if (*ptr == ':')
    {
        ptr++; // ':'
        char *pend = memchr(ptr, ' ', MAX_IRC+1);
        size_t plen = pend - ptr;
        //  |------------|V
        // :nnnn!uuuu@hhhh cccc...
        ptr[plen] = '\0';
        
        char *host = memchr(ptr, '@', plen);
        if (host != NULL)
        {
            size_t uplen = host - ptr;
            // |-------|
            // nnnn!uuuu@hhhh
            imsg->host = ptr+uplen+1;
            ptr[uplen] = '\0';

            char *user = memchr(ptr, '!', uplen);
            if (user != NULL)
            {
                size_t nlen = user - ptr;
                // |--|
                // nnnn!uuuu
                imsg->user = ptr+nlen+1;
                ptr[nlen] = '\0';
                imsg->nickname = ptr;
            }
            else
            {
                // nnnn@hhhh
                imsg->nickname = ptr;
                imsg->user = NULL;
            }
        }
        else
        {
            // nnnn
            imsg->nickname = ptr;
            imsg->user = imsg->host = NULL;
        }
        ptr = pend + 1;
    }
    else
    {
        imsg->nickname = imsg->user = imsg->host = NULL;
    }


    size_t cplen = end - ptr;
    char *params = memchr(ptr, ' ', cplen);
    imsg->command = ptr;
    if (params != NULL)
    {
        size_t clen = params - ptr;
        ptr = params;

        size_t rlen = cplen - clen;
        int n = 0;
        do
        {
            *ptr++ = '\0';
            if (*ptr == ':')
            {
                imsg->trailing = ++ptr;
                break;
            }
            if (n >= MAX_PARAMS)
            {
                imsg->trailing = ptr;
                break;
            }
            imsg->params[n++] = ptr;
        } while ((ptr = memchr(ptr, ' ', rlen)) != NULL);
    }
    
    return imsg;
}

char* irc_mtos(struct IrcMessage *imsg)
{
    int i;
    char *ret, *ptr, *end;

    if (imsg == NULL) return "\r\n";

    ret = ptr = malloc(sizeof(char[MAX_IRC+1]));
    end = ptr + (MAX_IRC-2);
    if (imsg->nickname != NULL)
    {
        if (ptr < end) *ptr++ = ':';
        ptr = memccpy(ptr, imsg->nickname, '\0', end-ptr) - 1;
        if (imsg->host != NULL)
        {
            if (imsg->user != NULL)
            {
                if (ptr < end) *ptr++ = '!';
                ptr = memccpy(ptr, imsg->user, '\0', end-ptr) - 1;
            }
            if (ptr < end) *ptr++ = '@';
            ptr = memccpy(ptr, imsg->host, '\0', end-ptr) - 1;
        }
    }
    if (ptr < end) *ptr++ = ' ';
    ptr = memccpy(ptr, imsg->command, '\0', end-ptr) - 1;
    for (i = 0; i <= MAX_PARAMS-1; i++)
    {
        if (imsg->params[i] == NULL) break;
        if (ptr < end) *ptr++ = ' ';
        ptr = memccpy(ptr, imsg->params[i], '\0', end-ptr) - 1;
    }
    if (imsg->trailing != NULL)
    {
        if (ptr < end) *ptr++ = ' ';
        if (ptr < end) *ptr++ = ':';
        ptr = memccpy(ptr, imsg->trailing, '\0', end-ptr) - 1;
    }
    memcpy(ptr, "\r\n", 3);
    return ret;
}

