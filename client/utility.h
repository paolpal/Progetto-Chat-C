#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "structs.h"

#ifndef _UTILITY_H_
#define _UTILITY_H_

struct user* append_user(struct user** head_ref, char* username);
ssize_t p_read_all(int fd, char* buffer, int b_size);
struct msg* create_my_msg(char* dest, char* text);

#endif
