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
#include "networking.h"
#include "constants.h"
#include "structs.h"

void display_help_message();
int login_check(char *user, char *pw);
int username_used(char *user);
int signup(char *user, char *pw);
int present(struct user_data** head_ref, char *username);
void update_register(struct user_data** head_ref, char *username, short port, int sd);
void push_registro(struct user_data** head_ref, char *username, short port, int sd);
void push_user(struct user_data** head_ref, struct user_data* user);
void delete_list(struct user_data** head_ref);
void display_list(struct user_data* head);
int login(struct user_data** head_ref, char *user, char *pw, short port, int sd);
int logout(struct user_data** head_ref, char *user);
int find_port(struct user_data** head_ref, char *username);
int is_online(struct user_data** head_ref, char *username);
struct chat* append_chat(struct chat** head_ref, char* username);
void push_chat(struct chat **l_chat_r, struct chat *chat);
struct hanging_msg** find_hanging_msg(struct chat** head_ref, char* username);
struct chat* find_chat(struct chat** head_ref, char* username);
void append_msg(struct hanging_msg** head_ref, char* dest_user, char* send_user, char* msg, int seq_n);
void push_msg(struct hanging_msg** l_hang_msg_r, struct hanging_msg* hang_msg);
void append_ack(struct msg_ack** l_ack_r, struct msg_ack* ack);
void prind_all_hanging_msg(struct chat* head);
void find_sender(struct hanging_msg* msg_head, struct sender** sender_head_ref);
void add_one_msg(struct sender** sender_head_ref, char* username);
void append_sender(struct sender** sender_head_ref, char* username);
struct hanging_msg* remove_msg(struct hanging_msg** l_msg_ref, char* sender);
void delete_by_socket(struct user_data** head_ref, int sd);
char* find_user_by_socket(struct user_data** head_ref, int sd);
void find_last_timestamp(time_t* timestamp, struct hanging_msg* l_msg, char* username);
int forward_msg(short port, char* sender, int seq_n, char* msg);
void forward_msg_ack(short port, char* dest, int seq_n);
// ***************** //
void save_register(struct user_data* _register);
void load_register(struct user_data** register_r);
void save_l_chat(struct chat* l_chat);
void load_l_chat(struct chat** l_chat_r);
void copy_hang_msg(struct hanging_msg* dest_r, struct hanging_msg* source_r);
void copy_ack(struct msg_ack* dest_r, struct msg_ack* source_r);
void copy_chat(struct chat* dest_r, struct chat* source_r);
void copy_user_data(struct user_data* dest_r, struct user_data* source_r);
struct msg_ack* create_ack(char* dest, int seq_n);
struct msg_ack* remove_ack(struct msg_ack** l_ack_r);

#endif
