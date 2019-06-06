CC = g++
CFLAGS = -g -Wall 
# -Wextra -Werror -D_REENTRANT -DCOLOR \
# 				 -D__BSD_VISIBLE -DREADLINE -Isupport -I.
LDFLAGS = -lpthread -lreadline

SRCS = node.cpp dbg.cpp lnxparse.cpp link.cpp routing.cpp

all: node 

node : $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f node 
