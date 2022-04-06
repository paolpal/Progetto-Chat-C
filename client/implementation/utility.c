#include "../utility.h"

void append_user(struct user** head_ref, char* username){
  struct user* new_user = (struct user*)malloc(sizeof(struct user));
  new_user->username = username;
  new_user->next = *head_ref;
  *head_ref = new_user;
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
