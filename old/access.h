#include "structs.h"

#ifndef __ACCESS__
#define __ACCESS__

int login_check(char user[20], char pw[20]){
  FILE *login_file = NULL;
  char username[20], password[20];
  int found = 0;
  if((login_file = fopen("login.txt","r")) == NULL){
    exit(1);
  }
  while(fscanf(login_file,"%s %s", &username[0], &password[0]) != EOF){
    if(strcmp(username,user)==0){
      if (strcmp(password,pw)==0)
        found = 1;
    }
  }
  fclose(login_file);
  return found;
}

int username_used(char user[20]){
  FILE *login_file = NULL;
  char username[20], password[20];
  int found = 0;
  if((login_file = fopen("login.txt","r")) == NULL){
    exit(1);
  }
  while(fscanf(login_file,"%s %s", &username[0], &password[0]) != EOF){
    if(strcmp(username,user)==0)
    found = 1;
  }
  fclose(login_file);
  return found;
}

int signup(char user[20], char pw[20]){
  FILE *login_file = NULL;
  if(username_used(user)) return 0;
  if ((login_file = fopen("login.txt","a")) == NULL){
    exit(1);
  }
  fprintf(login_file,"%s %s\n", user, pw);
  fclose(login_file);
  return 1;
}

int present(struct user_data** head_ref, char username[20]){
  struct user_data* current = *head_ref;
  int found = 0;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0)
      found = 1;
    current = current->next;
  }
  return found;
}

void push_registro(struct user_data** head_ref, char username[20], in_port_t port){
  if(!present(head_ref, username)){
    struct user_data* new_node = (struct user_data*) malloc(sizeof(struct user_data));
    strcpy(new_node->user_dest,username);
    new_node->port = port;
    new_node->next = (*head_ref);
    new_node->timestamp_login = (time_t*) malloc(sizeof(time_t));
    time(new_node->timestamp_login);
    printf("TEST 1\n");
    new_node->timestamp_logout = NULL;
    printf("TEST 2\n");
    (*head_ref) = new_node;
  }
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

void login(struct user_data** head_ref, char user[20], char pw[20]){
  if(login_check(user, pw)){
    push_registro(head_ref, user, htons(4242));
  }
}

void logout(struct user_data** head_ref, char user[20]){
  struct user_data* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->user_dest, user)==0){
      current->timestamp_logout = (time_t*) malloc(sizeof(time_t));
      time(current->timestamp_logout);
    }
    current = current->next;
  }
}

#endif
