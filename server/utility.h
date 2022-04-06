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

int login_check(char *user, char *pw);
int username_used(char *user);
int signup(char *user, char *pw);
int present(struct user_data** head_ref, char *username);
void reset_timestamp(struct user_data** head_ref, char *username);
void push_registro(struct user_data** head_ref, char *username, short port);
void delete_list(struct user_data** head_ref);
void display_list(struct user_data* head);
int login(struct user_data** head_ref, char *user, char *pw, short port);
int logout(struct user_data** head_ref, char *user);
int find_port(struct user_data** head_ref, char *username);
struct hanging_msg** append_dest(struct destinatario** head_ref, char* username);
struct hanging_msg** find_pending_msg(struct destinatario** head_ref, char* username);
void append_msg(struct hanging_msg** head_ref, char* dest_user, char* send_user, char* msg);
void prind_all_hanging_msg(struct destinatario* head);
void find_sender(struct hanging_msg* msg_head, struct sender** sender_head_ref);
void add_one_msg(struct sender** sender_head_ref, char* username);
void append_sender(struct sender** sender_head_ref, char* username);
struct hanging_msg* remove_msg(struct hanging_msg** l_msg_ref, char* sender);

#endif
