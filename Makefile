
CC=gcc
CFLAGS=-std=c11 -D_GNU_SOURCE -pthread -Wall -Wextra -Wpedantic -Werror -O0 \
        -Wno-error=pedantic -Wno-error=unused-parameter
LDFLAGS=-ggdb
LIBRARIES=
SOURCES= \
 main.c \
 log.c \
 conf.c \
 net.c \
 irc.c \
 util.c \
 queue.c
OBJECTS=$(SOURCES: .c=.o)
EXECUTABLE=kanra

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
		$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJECTS) $(LDFLAGS) $(LIBRARIES)

.c.o:
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
		$(RM) $(EXECUTABLE) *.o

rmr:
		$(MAKE) clean all
		./$(EXECUTABLE) irc.sublumin.al 6667

vg:
		$(MAKE) clean all
		valgrind --leak-check=full -v ./$(EXECUTABLE) irc.sublumin.al 6667

dbg:
		$(MAKE) clean all
		gdb -ex run --args ./$(EXECUTABLE) irc.sublumin.al 6667
