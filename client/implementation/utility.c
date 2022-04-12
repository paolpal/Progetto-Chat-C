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
    new_user->next_seq_n = 0;
    //new_user->next = *head_ref;
    new_user->next = NULL;
    *chatroom = new_user;
    return new_user;
  }
  else if(strcmp(username,(*chatroom)->username)==0) return NULL;
  else return append_user(&(*chatroom)->next, username);
}

void remove_user(struct user** chatroom, char* username){
  struct user* user = *chatroom, *prev;
  if(user != NULL && strcmp(user->username,username)==0) {
    *chatroom = user->next;
    free(user);
    return;
  }
  while(user != NULL && strcmp(user->username,username)!=0) {
    prev = user;
    user = user->next;
  }
  if(user == NULL) return;
  prev->next = user->next;
  free(user);
}

void print_chatroom(struct user* chatroom){
  struct user* user = chatroom;
  while(user!=NULL){
    printf("-) %s\n", user->username);
    user = user->next;
  }
}

/*
  struct user* è una lista di utenti con i quali sto chattando.
*/
int chatting_with(char *buffer, struct user* chatroom){
  struct user * c_user = chatroom;
  while (c_user!=NULL) {
    if(strcmp(c_user->username, buffer)==0) return 1;
    c_user= c_user->next;
  }
  return 0;
}

ssize_t p_read_all(int fd, char* buffer, int b_size){
  ssize_t nread;
  while((nread = read(fd, buffer, b_size)) > 0) {
    buffer[nread] = '\0';
  }

  return nread;
}

struct msg* create_my_msg(char* dest, char* text, int seq_n){
  struct msg* msg = (struct msg*)malloc(sizeof(struct msg));
  msg->sender = NULL;
  msg->ACK = 0;
  msg->dest = (char*) malloc((strlen(dest)+1)*sizeof(char));
  strcpy(msg->dest,dest);
  msg->text = (char*) malloc((strlen(text)+1)*sizeof(char));
  strcpy(msg->text, text);
  msg->seq_n = seq_n;
  return msg;
}

void send_chatroom_mp(int p_son_sd, struct user* chatroom){
  uint32_t len;
  struct user* user = chatroom;
  while(user!=NULL){
    len = strlen(user->username)+1;
    write(p_son_sd, &len, sizeof(len));
    write(p_son_sd, user->username, len);
    user=user->next;
  }
  len = 0;
  write(p_son_sd, &len, sizeof(len));
}

void display_help_message(){
  printf("Digita un comando: \n\n");
  printf("1) signup porta username password --> crea un account sul server\n");
  printf("2) in porta username password --> accede ad un account sul server\n");
  printf("3) hanging --> ricevo una lista degli utenti che mi hanno contattato\n");
  printf("4) show username --> ricevo i messaggi pendenti mandati da *username*\n");
  printf("5) chat username --> apro una chat con *username*\n");
  printf("6) share filename --> invio il file agli utenti con cui sto chattando\n");
  printf("7) out --> eseguo una disconnessione dal server\n");
  printf("8) esc --> termino l'applicazione\n");
  printf("8) help --> mostro il messaggio di aiuto\n");
}

struct msg* find_msg_list(struct chat **l_chat_ref, char *username){
  struct chat *c_chat = *l_chat_ref;
  while(c_chat!=NULL){
    if(strcmp(c_chat->user, username)==0){
      return c_chat->l_msg;
    }
    c_chat = c_chat->next;
  }
  return NULL;
}

void acknoledge_message(struct chat **l_chat_ref, char *username, int seq_n){
  struct msg* l_msg = find_msg_list(l_chat_ref, username);
  struct msg* c_msg = l_msg;
  while(c_msg!=NULL){
    if(c_msg->sender == NULL && c_msg->seq_n == seq_n) c_msg->ACK = 1;
    c_msg = c_msg->next;
  }
}

int is_in_addr_book(char* username){
  FILE *addr_book_file = NULL;
  char c_name[512];
  int found = 0;
  if((addr_book_file = fopen("rubrica.txt","r")) == NULL){
    exit(1);
  }
  while(!found && fscanf(addr_book_file,"%s", c_name) != EOF){
    if(strcmp(c_name, username)==0) found = 1;
  }
  fclose(addr_book_file);
  return found;
}

int is_online(int srv_sd, char* username){
  return online_check_protocol_client(srv_sd, username);
}
