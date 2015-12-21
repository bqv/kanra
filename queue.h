#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "irc.h"

struct MessageQueue
{
    struct IrcMessage *head;
    struct IrcMessage *last;

    unsigned int len;

    void (*push)(struct MessageQueue *this, struct IrcMessage *elem);
    struct IrcMessage* (*pop)(struct MessageQueue *this);
};

struct MessageQueue* new_msgqueue();
void mq_push(struct MessageQueue *this, struct IrcMessage *elem);
struct IrcMessage* mq_pop(struct MessageQueue *this);

#endif /*QUEUE_H*/
