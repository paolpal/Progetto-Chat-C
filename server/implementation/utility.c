#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../structs.h"
#include "../utility.h"

int login_check(char *user, char *pw){
  FILE *login_file = NULL;
  char username[512], password[512];
  int found = 0;
  if((login_file = fopen("login.txt","r")) == NULL){
    exit(1);
  }
  while(fscanf(login_file,"%s %s", username, password) != EOF){
    if(strcmp(username,user)==0){
      if (strcmp(password,pw)==0)
        found = 1;
    }
  }
  fclose(login_file);
  return found;
}

int username_used(char *user){
  FILE *login_file = NULL;
  char username[512], password[512];
  int found = 0;
  if((login_file = fopen("login.txt","r")) == NULL){
    exit(1);
  }
  while(fscanf(login_file,"%s %s", username, password) != EOF){
    if(strcmp(username,user)==0)
    found = 1;
  }
  fclose(login_file);
  return found;
}

int signup(char *user, char *pw){
  FILE *login_file = NULL;
  if(username_used(user)) return 0;
  if ((login_file = fopen("login.txt","a")) == NULL){
    exit(1);
  }
  fprintf(login_file,"%s %s\n", user, pw);
  fclose(login_file);
  return 1;
}

int present(struct user_data** head_ref, char *username){
  struct user_data* current = *head_ref;
  int found = 0;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0) found = 1;
    current = current->next;
  }
  return found;
}

int find_port(struct user_data** head_ref, char *username){
  struct user_data* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0 && current->timestamp_logout==NULL) return current->port;
    current = current->next;
  }
  return 0;
}

void reset_timestamp(struct user_data** head_ref, char *username){
  struct user_data* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0){
      free(current->timestamp_logout);
      current->timestamp_logout = NULL;
      time(current->timestamp_login);
      break;
    }
    current = current->next;
  }
}


// ATTENTO AL DOPPIO LOGIN
// se sono già loggato (timestamp_logout == NULL) allora non posso loggare un'altra volta (O si?) (Problema della doppia porta, il primo client non riceve più i messaggi)
void push_registro(struct user_data** head_ref, char *username, short port){
  if(!present(head_ref, username)){
    struct user_data* new_node = (struct user_data*) malloc(sizeof(struct user_data));
    new_node->user_dest = (char*) malloc(sizeof(char)*(strlen(username)+1));
    strcpy(new_node->user_dest,username);
    new_node->port = port;
    new_node->next = (*head_ref);
    new_node->timestamp_login = (time_t*) malloc(sizeof(time_t));
    time(new_node->timestamp_login);
    //printf("TEST 1\n");
    new_node->timestamp_logout = NULL;
    //printf("TEST 2\n");
    (*head_ref) = new_node;
  }
  // ALTRIMENTI RESETTO I TIMESTAMP
  reset_timestamp(head_ref, username);
}

void delete_list(struct user_data** head_ref) {
  struct user_data* current = *head_ref;
  struct user_data* next;

  while(current != NULL){
    next = current->next;
    free(current->timestamp_login);
    free(current->timestamp_logout);
    free(current);
    current = next;
  }
  *head_ref = NULL;
}

/*
void display_list(struct user_data* head) {
  struct user_data *temp;
  temp=head;
  while(temp!=NULL){
    if(temp->timestamp_logout != NULL)
      printf("%s %d %s %s", temp->user_dest, ntohs(temp->port), ctime(temp->timestamp_login), ctime(temp->timestamp_logout));
    else
      printf("%s %d %s", temp->user_dest, ntohs(temp->port), ctime(temp->timestamp_login));
    temp=temp->next;
  }
  return;
}
*/

void display_list(struct user_data* head) {
  struct user_data *temp;
  temp=head;
  while(temp!=NULL){
    if(temp->timestamp_logout == NULL)
      printf("%s*%d*%s", temp->user_dest, temp->port, ctime(temp->timestamp_login));
    temp=temp->next;
  }
  return;
}

int login(struct user_data** head_ref, char *user, char *pw, short port){
  if(login_check(user, pw)){
    push_registro(head_ref, user, port);
    return 1;
  }
  return 0;
}

int logout(struct user_data** head_ref, char *user){
  struct user_data* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->user_dest, user)==0){
      current->timestamp_logout = (time_t*) malloc(sizeof(time_t));
      time(current->timestamp_logout);
      return 1;
    }
    current = current->next;
  }
  return 0;
}

struct hanging_msg** find_pending_msg(struct destinatario** head_ref, char* username){
  struct destinatario* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->destinatario, username)==0) return &(current->messaggi);
    current = current->next;
  }
  return append_dest(head_ref, username);
}

struct hanging_msg** append_dest(struct destinatario** head_ref, char* username){
  struct destinatario* new_node = (struct destinatario*) malloc(sizeof(struct destinatario));
  new_node->destinatario = (char*) malloc(sizeof(char)*(strlen(username)+1));
  strcpy(new_node->destinatario, username);
  new_node->timestamp = NULL;
  new_node->messaggi = NULL;
  new_node->next = (*head_ref);
  (*head_ref) = new_node;
  return &(new_node->messaggi);
}

void append_msg(struct hanging_msg** head_ref, char* dest_user, char* send_user, char* msg){
  if(*head_ref==NULL){
    struct hanging_msg* new_msg = (struct hanging_msg*) malloc(sizeof(struct hanging_msg));
    new_msg->msg = msg;
    new_msg->send = send_user;
    new_msg->dest = dest_user;
    new_msg->next = *head_ref;
    *head_ref = new_msg;
  }
  else append_msg(&(*head_ref)->next,dest_user,send_user,msg);
}

void prind_all_hanging_msg(struct destinatario* head){
  struct destinatario* current_dest = head;
  struct hanging_msg* current_msg = NULL;
  while (current_dest!=NULL) {
    current_msg = current_dest->messaggi;
    printf("MESSAGGI PER %s\n", current_dest->destinatario);
    while (current_msg!=NULL) {
      printf("%s : %s", current_msg->send, current_msg->msg);
      current_msg = current_msg->next;
    }
    current_dest = current_dest->next;
  }
}

void append_sender(struct sender** sender_head_ref, char* username){
  struct sender* new_sender = (struct sender*)malloc(sizeof(struct sender));
  new_sender->username = username;
  new_sender->n_msg = 1;
  new_sender->next = *sender_head_ref;
  *sender_head_ref = new_sender;
}

/*
Aggiungo 1 al numero dei messaggi ricevuti dal mittente,
Se il mittente non è nella lista, lo aggiungo (1 messaggio già inserito)
*/
void add_one_msg(struct sender** sender_head_ref, char* username){
  struct sender* c_send = *sender_head_ref;
  while (c_send!=NULL){
    if(strcmp(c_send->username, username)==0){
      c_send->n_msg++;
      return;
    }
    c_send = c_send->next;
  }
  append_sender(sender_head_ref, username);
}

void find_sender(struct hanging_msg* msg_head, struct sender** sender_head_ref){
  struct hanging_msg* c_msg = msg_head;
  while(c_msg!=NULL){
    add_one_msg(sender_head_ref, c_msg->send);
    c_msg=c_msg->next;
  }
}

struct hanging_msg* remove_msg(struct hanging_msg** l_msg_ref, char* sender){
  struct hanging_msg* c_msg = *l_msg_ref, *prev;

  if(c_msg!=NULL && strcmp(c_msg->send,sender)==0){
    *l_msg_ref = c_msg->next;
    return c_msg;
  }

  while (c_msg != NULL && strcmp(c_msg->send,sender)!=0) {
      prev = c_msg;
      c_msg = c_msg->next;
  }
  if (c_msg != NULL) prev->next = c_msg->next;
  return c_msg;
}
