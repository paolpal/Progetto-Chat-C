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

void accoda_messaggio(struct chat **ricevuti, struct msg *messaggio);
void push_msg(struct msg **messaggi, struct msg *messaggio);
void send_msg(int cht_sd, char* my_user, char* msg, int seq_n);
void recv_msg(int srv_sd, int cht_sd, int p_father_sd, int p_son_sd, int chatting, char* my_user, struct chat** ricevuti, char* buffer);
void chat(int sd, int p_son_sd, int p_father_sd,char* my_user, char* dest_user);
void stampa_messaggio(struct msg *messaggio);
void print_chat(struct chat *l_chat, char* user);
void add_chat(struct chat **l_chat, char* user);

#endif
