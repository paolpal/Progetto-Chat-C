#include "structs.h"
#include "constants.h"
#include "protocols.h"
#include "utility.h"

#ifndef _CLIENT_CHAT_H_
#define _CLIENT_CHAT_H_

void accoda_messaggio(struct chat **ricevuti, struct msg *messaggio);
void push_msg(struct msg **messaggi, struct msg *messaggio);
void send_msg(int cht_sd, char* my_user, char* msg);
void recv_msg(int cht_sd, int p_father_sd, int p_son_sd, int chatting, struct chat** ricevuti, char* buffer);
void chat(int sd, int p_son_sd, int p_father_sd,char* my_user, char* dest_user);
void stampa_messaggio(struct msg *messaggio);
int chatting_with(char *buffer,struct user * chat_head);
void print_chat(struct chat *l_chat, char* user);
void add_chat(struct chat **l_chat, char* user);
void print_chatroom(struct user* chatroom);

#endif
