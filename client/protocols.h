#include "structs.h"

#ifndef _CLIENT_PROTOCOL_H_
#define _CLIENT_PROTOCOL_H_

int signup_protocol_client(int sd, char* user, char* pw);
int login_protocol_client(int sd, char* user, char* pw, short port);
int logout_protocol_client(int sd, char* user);
int new_chat_protocol_client(int sd, char* my_user, char* dest_user, struct sockaddr_in* dest_addr, char* msg);
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

#endif
