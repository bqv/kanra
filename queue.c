#include <stdlib.h>
#include "queue.h"

struct MessageQueue* new_msgqueue()
{
    struct MessageQueue *mq = malloc(sizeof(struct MessageQueue));
    mq->head = mq->last = NULL;
    mq->len = 0;
    mq->push = mq_push;
    mq->pop = mq_pop;
    return mq;
}

void mq_push(struct MessageQueue *this, struct IrcMessage *elem)
{
    if (this->head == NULL)
    {
        this->head = this->last = elem;
    }
    else
    {
        this->last->next = elem;
        this->last = elem;
    }
    this->last->next = NULL;
    this->len++;
}

struct IrcMessage* mq_pop(struct MessageQueue *this)
{
    struct IrcMessage* elem = this->head;
    if (elem != NULL)
    {
        this->head = elem->next;
        this->len--;
    }
    return elem;
}

