#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "structs.h"
#include "protocols.h"

struct user* append_user(struct user** chatroom_ref, char* username);
struct msg* create_my_msg(char* dest, char* my_username, char* text, int seq_n);
void print_chatroom(struct user* chatroom);
int chatting_with(char *buffer,struct user * chat_head);
void remove_user(struct user** chatroom, char* username);
void send_chatroom_mp(int p_son_sd, struct user* chatroom);
void display_help_message();
struct msg* find_msg_list(struct chat **l_chat_ref, char *username);
void acknoledge_message(struct chat **l_chat_ref, char *username, int seq_n);
int is_in_addr_book(char* username);
int is_online(int srv_sd, char* username);
int parametrs_num(char* str);
void load_chats(struct chat** l_chats_ref);
void save_chats(struct chat* l_chats);

#endif
