#ifndef _IRC_H_
#define _IRC_H_

#include "net.h"

#define MAX_IRC 512
#define MAX_PARAMS 14

struct IrcServer;

struct IrcMessage
{
    char _raw[MAX_IRC+1];
    size_t len;

    /* prefix */
    char *nickname; // or servername
    char *user;
    char *host;
    
    /* command */
    char *command;

    /* params */
    char *params[MAX_PARAMS];
    char *trailing;

    struct IrcMessage *next;
};

typedef void (*connect_fn_t)(struct IrcServer *this);
typedef void (*disconnect_fn_t)(struct IrcServer *this);
typedef void (*write_fn_t)(struct IrcServer *this, struct IrcMessage *);
typedef struct MessageQueue* (*read_fn_t)(struct IrcServer *this);

struct IrcServer
{
    struct Config *conf;
    struct Conn *conn;

    connect_fn_t connect;
    disconnect_fn_t disconnect;

    write_fn_t write;
    read_fn_t read;
};

struct IrcServer* new_server(struct Config *conf);
void irc_connect(struct IrcServer *server);
bool irc_handshake(struct IrcServer *server);
void irc_disconnect(struct IrcServer *server);

void irc_write(struct IrcServer *this, struct IrcMessage *message);
struct MessageQueue* irc_read(struct IrcServer *this);

struct IrcMessage* irc_stom(char *str, const size_t size);
char* irc_mtos(struct IrcMessage *imsg);

#endif /*IRC_H*/
