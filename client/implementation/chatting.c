#include "../chatting.h"

// *********************************************
// La funzione send_msg(...) invia alla socket
// specificata i dati relativi ad un messaggio
// viene invocata dal CHATTING PROCESS
// *********************************************
void send_msg(int cht_sd, char* my_user, char* msg, int seq_n){
  int len, ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  //INVIO LA RICHIESTA DI MESSAGGIO
  printf("<LOG-C> Invio richiesta di MESSAGE\n");
  sprintf(buffer, "MSG");
  ret = send(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL MITTENTE
  printf("<LOG-C> Invio lo USERNAME mittente (IO)\n");
  len = strlen(my_user)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //INVIO IL MITTENTE
  sprintf(buffer,"%s", my_user);
  ret = send(cht_sd, (void*) buffer, len, 0);

  //INVIO LA LUNGHEZZA DEL MESSAGGIO
  printf("<LOG-C> Invio il MESSAGGIO\n");
  len = strlen(msg)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //INVIO IL MESSAGGIO
  ret = send(cht_sd, (void*) msg, len, 0);

  //INVIO IL NUMERO DI SEQUENZA
  lmsg = htons(seq_n);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  return;
}

// *********************************************
// La funzione recv_msg(...) riceve il messaggio
// sulla socket specificata, avvia la procedura
// per l'invio dell MESSAGE ACK, contatta il
// CHATTING PROCESS per sapere se stampare il
// messaggio, e in ogni caso lo inserisce nella chat
// E' invocata dal MAIN PROCESS
// *********************************************
void recv_msg(int srv_sd, int cht_sd, int chatting, char* my_user, struct chat** l_chat_ref, struct user** chatroom_ref){
  uint16_t lmsg;
  int len;
  char buffer[BUF_LEN];

  struct chat* chat_p;
  struct msg* msg = (struct msg*) malloc(sizeof(struct msg));

  strncpy(msg->dest, my_user, S_BUF_LEN);

  //RICEVO LA LUNGHEZZA DEL MITTENTE
  printf("<LOG-M> Ricevo lo USERNAME mittente\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  msg->next = NULL;
  //RICEVO IL MITTENTE
  recv_all(cht_sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", msg->sender);

  //RICEVO LA LUNGHEZZA DEL MESSAGGIO
  printf("<LOG-M> Ricevo il MESSAGGIO\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //RICEVO IL MESSAGGIO
  recv_all(cht_sd, (void*)buffer, len, 0);
  strcpy(msg->text,buffer);

  //RICEVO IL NUMERO DI SEQUENZA
  printf("<LOG-M> Ricevo il NUMERO DI SEQUENZA\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  msg->seq_n = ntohs(lmsg);

  // INVIO L'ACK DI RICEZIONE
  send_msg_ack_protocol_client(srv_sd, my_user, msg->sender, msg->seq_n);

  if(chatting){
    if(chatting_with(msg->sender, *chatroom_ref))
      print_msg(msg, my_user);
  }
  // Inserisco il messaggio nella chat associata
  //add_msg(l_chat_ref, msg, my_user);
  chat_p = find_chat(l_chat_ref, msg->sender);
  push_msg(&(chat_p->l_msg), msg);
}

void add_msg(struct chat **l_chat, struct msg *msg, char* my_user){
  char* find = (strncmp(msg->sender, my_user, S_BUF_LEN)==0)? msg->dest:msg->sender;
  struct chat *c_chat = *l_chat;
  while(c_chat!=NULL){
    // cerco la chat associata all'utente specificato
    if(strncmp(c_chat->name, find, S_BUF_LEN)==0){
      push_msg(&c_chat->l_msg, msg);
      return;
    }
    c_chat = c_chat->next;
  }
  // se non ho trovato la chat, ne creo una apposita
  add_chat(l_chat, find);
  c_chat = *l_chat;
  push_msg(&c_chat->l_msg, msg);
}

// ******************************************
// Aggiungo una chat in testa alla lista
// ******************************************
struct chat* add_chat(struct chat **l_chat, char* user){
  int len;
  struct chat *new_chat  = (struct chat*) malloc(sizeof(struct chat));
  len = strlen(user)+1;
  strncpy(new_chat->name, user, S_BUF_LEN);
  new_chat->l_msg=NULL;
  new_chat->next_seq_n = 0;
  new_chat->next = (*l_chat);
  (*l_chat) = new_chat;
  return new_chat;
}

// *************************************
// data la lista di tutte le chat
// la funzione ritorna la chat
// corrispondente all'utente cercato
// *************************************
struct chat* find_chat(struct chat **l_chat_ref, char *username){
  struct chat *c_chat = *l_chat_ref;
  while(c_chat!=NULL){
    if(strcmp(c_chat->name, username)==0){
      return c_chat;
    }
    c_chat = c_chat->next;
  }
  return add_chat(l_chat_ref, username);
}

// ******************************************
// Inserisco un messaggio in fondo alla lista
// per facilitare la stampa
// ******************************************
void push_msg(struct msg **l_msg_r, struct msg *msg){
  if(*l_msg_r==NULL){
    msg->next = NULL;
    (*l_msg_r) = msg;
  }
  else push_msg(&(*l_msg_r)->next, msg);
}

void push_chat(struct chat **l_chat_r, struct chat *chat){
  if(*l_chat_r==NULL){
    chat->next = NULL;
    (*l_chat_r) = chat;
  }
  else push_chat(&(*l_chat_r)->next, chat);
}

// ******************************************
// stampo il messaggio nei formati:
// mittente : messaggio <-> messaggio ricevuto
// * messaggio <-> spedito, non consegnato
// ** messaggio <-> spedito e consegnato
// ******************************************
void print_msg(struct msg *msg, char* my_username){
  if(strncmp(msg->sender, my_username, S_BUF_LEN)!=0)
    printf("\r%s : %s", msg->sender, msg->text);
  else{
    if(msg->ACK==0) printf("\r* %s", msg->text);
    else printf("\r** %s", msg->text);
  }
  fflush(stdout);
}

// *********************************************
// Stampa ogni messaggio della chat specificata
// *********************************************
void print_chat(struct chat *l_chat, char* user, char* my_user){
  struct chat *c_chat = l_chat;
  struct msg *c_msg;
  while(c_chat!=NULL){
    if(strcmp(c_chat->name, user)==0){
      c_msg = c_chat->l_msg;
      while(c_msg!=NULL){
        print_msg(c_msg, my_user);
        c_msg = c_msg->next;
      }
    }
    c_chat = c_chat->next;
  }
}

void copy_msg(struct msg *dest_msg_r, struct msg* source_msg_r){
  strncpy(dest_msg_r->text, source_msg_r->text, BUF_LEN);
  strncpy(dest_msg_r->sender, source_msg_r->sender, S_BUF_LEN);
  strncpy(dest_msg_r->dest, source_msg_r->dest, S_BUF_LEN);
  dest_msg_r->ACK = source_msg_r->ACK;
  dest_msg_r->seq_n = source_msg_r->seq_n;
}

void copy_chat(struct chat *dest_chat_r, struct chat* source_chat_r){
  strncpy(dest_chat_r->name, source_chat_r->name, S_BUF_LEN);
}

// *************************************
// Elimino la lista delle chat
// *************************************
void delete_l_chat(struct chat **l_chat_r){
  struct chat* c_chat = *l_chat_r;
  struct chat* next;

  while(c_chat!=NULL){
    next = c_chat->next;
    delete_l_msg(&(c_chat->l_msg));
    free(c_chat);
    c_chat = next;
  }

  *l_chat_r = NULL;
}

// *************************************
// Elimino la lista dei messaggi
// *************************************
void delete_l_msg(struct msg **l_msg_r){
  struct msg* c_msg = *l_msg_r;
  struct msg* next;

  while(c_msg!=NULL){
    next = c_msg->next;
    free(c_msg);
    c_msg = next;
  }

  *l_msg_r = NULL;
}
