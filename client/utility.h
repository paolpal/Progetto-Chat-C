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
#include "protocols.h"

#ifndef _UTILITY_H_
#define _UTILITY_H_

struct user* append_user(struct user** head_ref, char* username);
ssize_t p_read_all(int fd, char* buffer, int b_size);
struct msg* create_my_msg(char* dest, char* text, int seq_n);
void print_chatroom(struct user* chatroom);
int chatting_with(char *buffer,struct user * chat_head);
void remove_user(struct user** chatroom, char* username);
void send_chatroom_mp(int p_son_sd, struct user* chatroom);
void display_help_message();
struct msg* find_msg_list(struct chat **l_chat_ref, char *username);
void acknoledge_message(struct chat **l_chat_ref, char *username, int seq_n);
int is_in_addr_book(char* username);
int is_online(int srv_sd, char* username);

#endif
