#include "structs.h"

#ifndef _PROTOCOLS_H_
#define _PROTOCOLS_H_

void signup_protocol(int i, char* buffer);
void login_protocol(int i, struct user_data** utenti, struct chat** l_char_r, char* buffer);
//void login_protocol(int i, struct user_data** head_ref, char* buffer);
void logout_protocol(int i, struct user_data** head_ref, char* buffer);
void new_chat_protocol(int i, struct user_data** utenti, struct chat** destinatari,  char* buffer);
void hanging_protocol(int i, struct chat** destinatari, char* buffer);
void show_protocol(int i, struct chat** destinatari, char* buffer);
void group_protocol(int i, struct user_data** utenti, char* buffer);
void forw_msg_ack_protocol(int i, struct user_data** utenti, struct chat** l_chat_r, char* buffer);
//void forw_msg_ack_protocol(int i, struct user_data** utenti, char* buffer);
void online_check_protocol(int i, struct user_data** utenti, char* buffer);

#endif
