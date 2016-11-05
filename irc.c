#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "util.h"
#include "log.h"
#include "irc.h"

struct IrcServer* new_server(struct Config *conf)
{
    struct IrcServer *server = calloc(1, sizeof(struct IrcServer));

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
    bool registered = false;

    this->conn->send(this->conn, "NICK kanra\r\n");
    this->conn->send(this->conn, "USER kanra * * me\r\n");

    while (!registered)
    {
        struct MessageQueue *imsgs = this->read(this);

        struct IrcMessage *imsg;
        for (imsg = imsgs->head; imsg != NULL; imsg = imsg->next)
        {
            if (!strcmp(imsg->command, "PING"))
            {
                struct IrcMessage reply = {0};
                reply.command = "PONG";
                reply.trailing = imsg->trailing;

                this->write(this, &reply);
            }
            else if (!strcmp(imsg->command, "001"))
            {
                registered = true;
            }

            char *msgstr = irc_mtos(imsg);
            wlogf(INFO, "Got:\n%s\n", msgstr);

            free(imsg);
            free(msgstr);
        }
    }
}

void irc_write(struct IrcServer *this, struct IrcMessage *message)
{
    struct Conn *conn = this->conn;
    char *line = irc_mtos(message);
    conn->send(conn, line);
    free(line);
}

struct MessageQueue *irc_read(struct IrcServer *this)
{
    struct MessageQueue *mq = new_msgqueue();
    struct IrcMessage *imsg;
    char *line, *end;

    this->conn->recv(this->conn);
    line = calloc(MAX_IRC+1, sizeof(char));

    while ((end = memccpy(line, this->conn->buffer, '\n', MAX_IRC)) != NULL)
    {
        *end = '\0';

        imsg = irc_stom(line, end-line);

        if (imsg != NULL)
        {
            this->conn->bcur -= imsg->len;
            memmove(this->conn->buffer, this->conn->buffer+imsg->len, sizeof(this->conn->buffer)-this->conn->bcur);
        }
        mq->push(mq, imsg);
    }
    free(line);
    return mq;
}

struct IrcMessage *irc_stom(char *str, const size_t size)
{
    char *ptr, *end;

    struct IrcMessage *imsg = calloc(1, sizeof(struct IrcMessage));
    memccpy(imsg->_raw, str, '\0', size);
    imsg->len = size;

    ptr = imsg->_raw;
    end = ptr+imsg->len;
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
    if (imsg->trailing != NULL)
    {
        ptr = memchr(ptr, '\r', end-ptr);
        if (*(ptr+1) == '\n')
        {
            *ptr = *(ptr+1) = '\0';
        }
    }
    
    return imsg;
}

char *irc_mtos(struct IrcMessage *imsg)
{
    int i;
    char *ret, *ptr;

    // Null filter
    if (imsg == NULL) return "\r\n";

    // Allocate message, mark end
    ret = ptr = calloc(MAX_IRC+1, sizeof(char));
    const char *end = ptr + (MAX_IRC-2);

    if (imsg->nickname != NULL)
    {
        if (ptr < end) *ptr++ = ':';
        if (ptr < end) ptr = memccpy(ptr, imsg->nickname, '\0', end-ptr) - 1;
        if (imsg->host != NULL)
        {
            if (imsg->user != NULL)
            {
                if (ptr < end) *ptr++ = '!';
                if (ptr < end) ptr = memccpy(ptr, imsg->user, '\0', end-ptr) - 1;
            }
            if (ptr < end) *ptr++ = '@';
            if (ptr < end) ptr = memccpy(ptr, imsg->host, '\0', end-ptr) - 1;
        }
        if (ptr < end) *ptr++ = ' ';
    }

    if (imsg->command == NULL) return "\r\n";
    else if (ptr < end) ptr = memccpy(ptr, imsg->command, '\0', end-ptr) - 1;

    for (i = 0; i <= MAX_PARAMS-1; i++)
    {
        if (imsg->params[i] == NULL) break;
        if (ptr < end) *ptr++ = ' ';
        if (ptr < end) ptr = memccpy(ptr, imsg->params[i], '\0', end-ptr) - 1;
    }

    if (imsg->trailing != NULL)
    {
        if (ptr < end) *ptr++ = ' ';
        if (ptr < end) *ptr++ = ':';
        if (ptr < end) ptr = memccpy(ptr, imsg->trailing, '\0', end-ptr) - 1;
    }
    if (ptr < end) memcpy(ptr, "\r\n", 3);
    return ret;
}

