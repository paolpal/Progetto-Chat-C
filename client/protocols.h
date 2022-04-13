#ifndef _CLIENT_PROTOCOL_H_
#define _CLIENT_PROTOCOL_H_

#include "structs.h"
#include "chatting.h"
#include "constants.h"
#include "networking.h"
#include "filetransfer.h"

int signup_protocol_client(int sd, char* user, char* pw);
int login_protocol_client(int sd, char* user, char* pw, short port);
int logout_protocol_client(int sd, char* user);
int new_chat_protocol_client(int sd, char* my_user, char* dest_user, struct sockaddr_in* dest_addr, char* msg, int* seq_n);
void hanging_protocol_client(int sd, char* user);
void show_protocol_client(int sd, char* my_user, char* sender_user, struct chat** l_chat);
void receive_file_protocol_client(int sd);
void send_file_protocol_client(struct sockaddr_in* dest_addr, char* filename);
void group_protocol_client(int sd);
void add_user_request_protocol_client(int cht_sd, char* username);
void add_user_protocol_client(int sd, int p_father_sd);
void leave_chatroom_request_protocol_client(int cht_sd, char* my_username);
void leave_chatroom_protocol_client(int sd, int p_father_sd);
void join_chatroom_request_protocol_client(int cht_sd, char* my_username, struct user** chatroom_ref);
void join_chatroom_protocol_client(int sd, int p_father_sd, int p_son_sd);
void send_msg_ack_protocol_client(int srv_sd, char *my_user, char *sender, int seq_n);
void recv_msg_ack_protocol_client(int sd, struct chat** l_chat_ref);
int online_check_protocol_client(int srv_sd, char* username);

#endif
