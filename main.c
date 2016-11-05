/* Kanra IRC Bot */

#include <assert.h>
#include <stdlib.h>
#include "conf.h"
#include "log.h"
#include "irc.h"

#define VERSION "0.1.7"

int main(int argc, char *argv[])
{
    wlogf(INFO, "Starting Kanra (" VERSION ")\n");

    struct Config *conf;
    struct IrcServer *server;

    if ((conf = parse_args(argc, argv)) != NULL)
    {
        server = new_server(conf);

        /* Loop */
        server->connect(server);

        wlogf(INFO, "Finishing\n");

        server->disconnect(server);
        free(server);
        free(conf);
    }

    return EXIT_SUCCESS;
}
