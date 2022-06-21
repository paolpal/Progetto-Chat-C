#include "../utility.h"

// *********************************************
// Aggiunge l'utente specificato alla chatroom
// *********************************************
struct user* append_user(struct user** chatroom, char* username){
  struct user* new_user;
  if(*chatroom==NULL){
    new_user = (struct user*)malloc(sizeof(struct user));
    strcpy(new_user->name, username);
    new_user->cht_sd = 0;
    new_user->next = NULL;
    *chatroom = new_user;
    return new_user;
  }
  else if(strcmp(username,(*chatroom)->name)==0) return NULL;
  else return append_user(&(*chatroom)->next, username);
}

// *********************************************
// Rimuove l'utente specificato dalla chatroom
// *********************************************
struct user* remove_user(struct user** chatroom, char* username){
  struct user* user = *chatroom, *prev;
  if(user != NULL && strcmp(user->name,username)==0) {
    *chatroom = user->next;
    return user;
  }
  while(user != NULL && strcmp(user->name,username)!=0) {
    prev = user;
    user = user->next;
  }
  if(user != NULL) prev->next = user->next;
  return user;
}

// *********************************************
// Stampa l'elenco dei partecipanti alla chatroom
// *********************************************
void print_chatroom(struct user* chatroom){
  struct user* user = chatroom;
  while(user!=NULL){
    printf("-) %s\n", user->name);
    user = user->next;
  }
}

// *************************************
// Elimino la lista chatroom
// *************************************
void delete_chatroom(struct user** chatroom_ref){
  struct user* c_user = *chatroom_ref;
  struct user* next;

  while(c_user!=NULL){
    next = c_user->next;
    free(c_user);
    c_user = next;
  }

  *chatroom_ref = NULL;
}

// *************************************
// Controllo che l'utente specificato
// faccia parte della chatroom
// *************************************
int chatting_with(char *user, struct user* chatroom){
  struct user * c_user = chatroom;
  while (c_user!=NULL) {
    if(strcmp(c_user->name, user)==0) return 1;
    c_user= c_user->next;
  }
  return 0;
}

// *************************************
// passo i dati necessari alla creazione
// della struttura msg.
// Il mittente è NULL per identificare
// che è l'utente stesso...
// *************************************
struct msg* create_my_msg(char* dest, char* my_username, char* text, int seq_n){
  struct msg* msg = (struct msg*)malloc(sizeof(struct msg));
  msg->ACK = 0;
  strncpy(msg->dest, dest, S_BUF_LEN);
  strncpy(msg->sender, my_username, S_BUF_LEN);
  strncpy(msg->text, text, BUF_LEN);
  msg->seq_n = seq_n;
  return msg;
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
  printf("9) help --> mostro il messaggio di aiuto\n");
}

// *************************************
// data la lista di tutte le chat
// la funzione ritorna la lista dei messaggi
// corrispondente all'utente cercato
// *************************************
struct msg* find_msg_list(struct chat *l_chat, char *username){
  struct chat *c_chat = l_chat;
  while(c_chat!=NULL){
    if(strcmp(c_chat->name, username)==0){
      return c_chat->l_msg;
    }
    c_chat = c_chat->next;
  }
  return NULL;
}

// *************************************
// dato lo username di destinazione e
// il numero di sequenza, setto l'ACK di
// consegna al messaggio corretto
// *************************************
void acknoledge_message(struct chat **l_chat_ref, char *username, int seq_n){
  struct msg* l_msg = find_msg_list(*l_chat_ref, username);
  struct msg* c_msg = l_msg;
  while(c_msg!=NULL){
    //if(c_msg->sender == NULL && c_msg->seq_n == seq_n) c_msg->ACK = 1;
    if(strcmp(c_msg->dest,username)==0 && c_msg->seq_n == seq_n) c_msg->ACK = 1;
    c_msg = c_msg->next;
  }
}

// *************************************
// controllo che lo username specificato
// sia nella rubrica dell'utente, altrimenti non
// posso aprire la chat e ritorno 0
// *************************************
int is_in_addr_book(char* username, char* c_user){
  FILE *addr_book_file = NULL;
  char c_name[512];
  char* filename;
  int found = 0;
  int len;

  len = strlen(c_user)+13;
  filename = (char*)malloc(len*sizeof(char));
  sprintf(filename,"%s_rubrica.txt", c_user);
  addr_book_file = fopen(filename,"r");
  free(filename);

  if(addr_book_file == NULL){
    exit(1);
  }
  while(!found && fscanf(addr_book_file,"%s", c_name) != EOF){
    if(strcmp(c_name, username)==0) found = 1;
  }
  fclose(addr_book_file);
  return found;
}

// *************************************
// per verificare che l'utente specificato
// sia online, contact il server con
// l'apposita procedura
// *************************************
int is_online(int srv_sd, char* username){
  return online_check_protocol_client(srv_sd, username);
}

// *************************************
// la funzione conta i parametri passati
// da linea di comando.
// necessaria per non avere prblemi
// durante il parsing dei comandi
// *************************************
int parametrs_num(char* str){
  int count = 0;
  char *ptr = str;
  while((ptr = strchr(ptr, ' ')) != NULL) {
      count++;
      ptr++;
  }
  return count;
}

void save_chats(struct chat* l_chats, char* my_user){
  FILE* chat_file_p;
  FILE* msg_file_p;
  char *filename;

  int len;

  struct chat* next_chat;
  struct msg* next_msg;

  struct chat* c_chat = l_chats;

  len = strlen(my_user)+11;
  filename = (char*)malloc(len*sizeof(char));
  sprintf(filename,"%s_chats.dat", my_user);
  chat_file_p = fopen (filename, "wb");
  free(filename);
  if(chat_file_p == NULL){
    return;
  }

  printf("<LOG> Aperto il file CHATS.DAT.\n");
  while(c_chat!=NULL){
    len = strlen(c_chat->name)+strlen(my_user)+6;
    filename = (char*)malloc(len*sizeof(char));
    sprintf(filename,"%s_%s.dat", my_user, c_chat->name);
    msg_file_p = fopen (filename, "wb");
    free(filename);
    if(msg_file_p != NULL){
      struct msg *c_msg = c_chat->l_msg;
      while(c_msg!=NULL){
        next_msg = c_msg->next;
        c_msg->next = NULL;
        fwrite (c_msg, sizeof(struct msg), 1, msg_file_p);
        c_msg = next_msg;
      }
      fclose(msg_file_p);
      printf("<LOG> Salvata la chat con: %s.\n",c_chat->name);
    }
    next_chat = c_chat->next;
    c_chat->next = NULL;
    c_chat->l_msg = NULL;
    fwrite (c_chat, sizeof(struct chat), 1, chat_file_p);
    c_chat = next_chat;
  }
  fclose(chat_file_p);
}

void load_chats(struct chat** l_chats_ref, char* my_user){
  FILE* chat_file_p;
  FILE* msg_file_p;
  char *filename;

  int len;

  struct chat in_chat;
  struct chat* new_chat;
  struct msg in_msg;
  struct msg* new_msg;

  len = strlen(my_user)+11;
  filename = (char*)malloc(len*sizeof(char));
  sprintf(filename,"%s_chats.dat", my_user);
  chat_file_p = fopen (filename, "rb");
  free(filename);
  if(chat_file_p == NULL){
    return;
  }

  while(fread(&in_chat, sizeof(struct chat), 1, chat_file_p)){
    len = strlen(in_chat.name)+strlen(my_user)+6;
    filename = (char*)malloc(len*sizeof(char));
    sprintf(filename, "%s_%s.dat", my_user, in_chat.name);
    msg_file_p = fopen (filename, "rb");
    free(filename);

    if(msg_file_p!=NULL){
      new_chat = (struct chat*) malloc(sizeof(struct chat));
      copy_chat(new_chat, &in_chat);

      while(fread(&in_msg, sizeof(struct msg), 1, msg_file_p)){
        new_msg = (struct msg*) malloc(sizeof(struct msg));
        copy_msg(new_msg, &in_msg);
        push_msg(&new_chat->l_msg, new_msg);
      }
      fclose(msg_file_p);
    }
    push_chat(l_chats_ref, new_chat);

  }
  fclose(chat_file_p);
}

int lenght(struct user* chatroom){
  struct user* c_user = chatroom;
  int count = 0;

  while(c_user!=NULL){
    count++;
    c_user = c_user->next;
  }

  return count;
}
