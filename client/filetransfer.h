#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef _FILETRANSFER_H_
#define _FILETRANSFER_H_

#include "constants.h"
#include "protocols.h"

void recv_file(int sockfd, char* filename);
void send_file(char* filename, int sockfd);
void recv_file_b(int sockfd, char* filename);
void send_file_b(char* filename, int sockfd);

#endif
