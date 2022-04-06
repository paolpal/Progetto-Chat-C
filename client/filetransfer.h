#include "constants.h"
#include "protocols.h"

#ifndef _FILETRANSFER_H_
#define _FILETRANSFER_H_

void recv_file(int sockfd, char* filename);
void send_file(char* filename, int sockfd);
void recv_file_b(int sockfd, char* filename);
void send_file_b(char* filename, int sockfd);

#endif
