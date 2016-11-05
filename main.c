/* Kanra IRC Bot */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"
#include "conf.h"
#include "log.h"
#include "irc.h"

#define VERSION "0.2.9"

void loop(struct IrcServer *server)
{
    struct MessageQueue *imsgs;
    struct IrcMessage *imsg;

    struct IrcMessage joinmsg = {.command = "JOIN", .trailing = "#programming"};
    server->write(server, &joinmsg);

    for (;;) while ((imsgs = server->read(server)) != NULL)
    {
        while ((imsg = imsgs->pop(imsgs)) != NULL)
        {
            if (!strcmp(imsg->command, "PING"))
            {
                struct IrcMessage reply = {0};
                reply.command = "PONG";
                reply.trailing = imsg->trailing;

                server->write(server, &reply);
            }
            else if (!strcmp(imsg->command, "ERROR"))
            {
                wlogf(INFO, "Finishing\n");
                free(imsg);
                while ((imsg = imsgs->pop(imsgs)) != NULL) free(imsg);
                free(imsgs);
                return;
            }
            else if (!strcmp(imsg->command, "PRIVMSG"))
            {
                struct IrcMessage reply = *imsg;
                reply.nickname = NULL;

                server->write(server, &reply);
            }

            char *msgstr = irc_mtos(imsg);
            wlogf(INFO, "Got:\n%s\n", msgstr);

            imsgs->head = imsg->next;
            free(imsg);
            free(msgstr);
        }
        free(imsgs);
    }
}

int main(int argc, char *argv[])
{
    wlogf(INFO, "Starting Kanra (" VERSION ")\n");

    struct Config *conf;
    struct IrcServer *server;

    if ((conf = parse_args(argc, argv)) != NULL)
    {
        server = new_server(conf);

        server->connect(server);

        loop(server);

        server->disconnect(server);

        free(server);
        free(conf);
    }

    return EXIT_SUCCESS;
}
