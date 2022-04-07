#include "../utility.h"

struct user* append_user(struct user** chatroom, char* username){
  int len;
  struct user* new_user;
  if(*chatroom==NULL){
    new_user = (struct user*)malloc(sizeof(struct user));
    len = strlen(username)+1;
    new_user->username = (char*) malloc(len*sizeof(char));
    strcpy(new_user->username, username);
    new_user->cht_sd = 0;
    //new_user->next = *head_ref;
    new_user->next = NULL;
    *chatroom = new_user;
    return new_user;
  }
  else if(strcmp(username,(*chatroom)->username)==0) return NULL;
  else return append_user(&(*chatroom)->next, username);
}

ssize_t p_read_all(int fd, char* buffer, int b_size){
  ssize_t nread;
  while((nread = read(fd, buffer, b_size)) > 0) {
    buffer[nread] = '\0';
  }

  return nread;
}

struct msg* create_my_msg(char* dest, char* text){
  struct msg* msg = (struct msg*)malloc(sizeof(struct msg));
  msg->sender = NULL;
  msg->dest = (char*) malloc((strlen(dest)+1)*sizeof(char));
  strcpy(msg->dest,dest);
  msg->text = (char*) malloc((strlen(text)+1)*sizeof(char));
  strcpy(msg->text, text);
  return msg;
}
