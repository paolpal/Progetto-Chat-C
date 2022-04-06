#include "structs.h"

#ifndef _PROTOCOLS_H_
#define _PROTOCOLS_H_

void signup_protocol(int i, char* buffer);
void login_protocol(int i, struct user_data** head_ref, char* buffer);
void logout_protocol(int i, struct user_data** head_ref, char* buffer);
void new_chat_protocol(int i, struct user_data** utenti, struct destinatario** destinatari,  char* buffer);
int forward_msg(short port, char* sender, char* msg);
void hanging_protocol(int i, struct destinatario** destinatari, char* buffer);
void show_protocol(int i, struct destinatario** destinatari, char* buffer);
void group_protocol(int i, struct user_data** utenti, char* buffer);

#endif
