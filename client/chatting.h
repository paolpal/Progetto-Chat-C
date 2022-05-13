#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef _CLIENT_CHAT_H_
#define _CLIENT_CHAT_H_

#include "structs.h"
#include "constants.h"
#include "protocols.h"
#include "utility.h"

void send_msg(int cht_sd, char* my_user, char* msg, int seq_n);
void recv_msg(int srv_sd, int cht_sd, int chatting, char* my_user, struct chat** ricevuti, struct user** chatroom_ref);
void add_msg(struct chat **l_chat_r, struct msg *msg, char* my_user);
struct chat* find_chat(struct chat **l_chat_ref, char *username);
struct chat* add_chat(struct chat **l_chat_r, char* user);
void push_msg(struct msg **l_msg_r, struct msg *msg);
void push_chat(struct chat **l_chat, struct chat *chat);
void print_msg(struct msg *msg, char* my_user);
void print_chat(struct chat *l_chat, char* user, char* my_user);
void copy_msg(struct msg *dest_msg_r, struct msg* source_msg_r);
void copy_chat(struct chat *dest_chat_r, struct chat* source_chat_r);

#endif
