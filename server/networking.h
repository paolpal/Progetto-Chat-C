#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef _NETWORKING_H_
#define _NETWORKING_H_

int send_all(int socket, void *buffer, size_t length, int flags);
int recv_all(int socket, void *buffer, size_t length, int flags);

#endif
