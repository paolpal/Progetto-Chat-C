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
void recv_msg(int srv_sd, int cht_sd, int p_father_sd, int p_son_sd, int chatting, char* my_user, struct chat** ricevuti, char* buffer){
  uint16_t lmsg;
  uint32_t len_t;
  int len;
  struct msg* msg = (struct msg*) malloc(sizeof(struct msg));

  msg->dest = NULL;

  //RICEVO LA LUNGHEZZA DEL MITTENTE
  printf("<LOG-M> Ricevo lo USERNAME mittente\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  msg->sender = (char*) malloc(len*sizeof(char));
  msg->next = NULL;
  //RICEVO IL MITTENTE
  recv_all(cht_sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", msg->sender);

  //RICEVO LA LUNGHEZZA DEL MESSAGGIO
  printf("<LOG-M> Ricevo il MESSAGGIO\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  msg->text = (char*) malloc(len*sizeof(char));
  //RICEVO IL MESSAGGIO
  recv_all(cht_sd, (void*)buffer, len, 0);
  strcpy(msg->text,buffer);

  //RICEVO IL NUMERO DI SEQUENZA
  printf("<LOG-M> Ricevo il NUMERO DI SEQUENZA\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  msg->seq_n = ntohs(lmsg);

  // INVIO L'ACK DI RICEZIONE
  send_msg_ack_protocol_client(srv_sd, my_user, msg->sender, msg->seq_n);

  //COMUNICO CON IL CHATTING PROCESS SE ATTIVO
  if(chatting){
    if(chatting_with(msg->sender, chatroom))
      print_msg(msg);
  }
  // Inserisco il messaggio nella chat associata
  add_msg(ricevuti, msg);

}

void add_msg(struct chat **l_chat, struct msg *msg){
  char* find = (msg->sender==NULL)? msg->dest:msg->sender;
  struct chat *c_chat = *l_chat;
  while(c_chat!=NULL){
    // cerco la chat associata all'utente specificato
    // se il messaggio lo ho scritto io msg->sender sarà NULL
    if(strcmp(c_chat->user, find)==0){
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
void add_chat(struct chat **l_chat, char* user){
  int len;
  struct chat *new_chat  = (struct chat*) malloc(sizeof(struct chat));
  len = strlen(user)+1;
  new_chat->user = (char*) malloc(len * sizeof(char));
  strcpy(new_chat->user, user);
  new_chat->l_msg=NULL;
  new_chat->next = (*l_chat);
  (*l_chat) = new_chat;
}

void push_chat(struct chat **l_chat, struct chat *chat){
  if(*l_chat==NULL){
    chat->next = (*l_chat);
    (*l_chat) = chat;
  }
  else push_chat(&(*l_chat)->next, chat);
}

// ******************************************
// Inserisco un messaggio in fondo alla lista
// per facilitare la stampa
// ******************************************
void push_msg(struct msg **l_msg, struct msg *msg){
  if(*l_msg==NULL){
    msg->next = (*l_msg);
    (*l_msg) = msg;
  }
  else push_msg(&(*l_msg)->next, msg);
}

// ******************************************
// stampo il messaggio nei formati:
// mittente : messaggio <-> messaggio ricevuto
// * messaggio <-> spedito, non consegnato
// ** messaggio <-> spedito e consegnato
// ******************************************
void print_msg(struct msg *msg){
  if(msg->sender != NULL)
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
void print_chat(struct chat *l_chat, char* user){
  struct chat *c_chat = l_chat;
  struct msg *c_msg;
  while(c_chat!=NULL){
    if(strcmp(c_chat->user, user)==0){
      c_msg = c_chat->l_msg;
      while(c_msg!=NULL){
        print_msg(c_msg);
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

void copy_chat(struct chat *dest_chat_r, struct msg* source_chat_r){
  strncpy(dest_msg_r->name, source_msg_r->name, S_BUF_LEN);
}
