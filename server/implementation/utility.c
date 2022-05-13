#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../utility.h"

// ****************************************
// la funzione login_check(...) controlla che la
// coppia username e password passata sia
// presente nel file login.txt
// ****************************************
int login_check(char *user, char *pw){
  FILE *login_file = NULL;
  char username[512], password[512];
  int found = 0;
  if((login_file = fopen("login.txt","r")) == NULL){
    exit(1);
  }
  while(!found && fscanf(login_file,"%s %s", username, password) != EOF){
    if(strcmp(username,user)==0){
      if (strcmp(password,pw)==0)
        found = 1;
    }
  }
  fclose(login_file);
  return found;
}

// ****************************************
// la funzione username_used(...) verifica
// che lo username passato non sia
// gia' stato utilizzato
// ****************************************
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

// ****************************************
// la funzione signup(...) aggiunge un record
// al file login.txt
// ****************************************
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

// ****************************************
// la funzione present(...) controlla che
// lo username specificato sia o meno nel
// registro passato
// ****************************************
int present(struct user_data** head_ref, char *username){
  struct user_data* current = *head_ref;
  int found = 0;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0 && current->timestamp_logout!=NULL) found = 1;
    current = current->next;
  }
  return found;
}

// ****************************************************************
// la funzione find_port(...) consultando il registro passato
// restituisce la porta associata allo username specificato
// ****************************************************************
int find_port(struct user_data** head_ref, char *username){
  struct user_data* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0 && current->timestamp_logout==NULL) return current->port;
    current = current->next;
  }
  return 0;
}

// ****************************************
// la funzione is_online(...) controlla che
// l'utente specificato sia online consultando
// il registro
// ****************************************
int is_online(struct user_data** l_user_ref, char *username){
  struct user_data* c_user = *l_user_ref;
  while(c_user != NULL){
    if(strcmp(c_user->user_dest, username)==0 && c_user->timestamp_logout==NULL) return 1;
    c_user = c_user->next;
  }
  return 0;
}

// ****************************************
// la funzione update_register(...) aggiorna i dati del registro
// dello username specificato
// ****************************************
void update_register(struct user_data** head_ref, char *username, short port, int sd){
  struct user_data* current = *head_ref;
  while(current != NULL){
    if(strcmp(current->user_dest, username)==0){
      free(current->timestamp_logout);
      current->timestamp_logout = NULL;
      time(current->timestamp_login);
      current->port = port;
      current->sd = sd;
      break;
    }
    current = current->next;
  }
}


// ATTENTO AL DOPPIO LOGIN
// se sono già loggato (timestamp_logout == NULL) allora non posso loggare un'altra volta (O si?)
// (Problema della doppia porta, il primo client non riceve più i messaggi)
//
void push_registro(struct user_data** head_ref, char *username, short port, int sd){
  if(!present(head_ref, username)){
    struct user_data* new_node = (struct user_data*) malloc(sizeof(struct user_data));
    strncpy(new_node->user_dest,username, S_BUF_LEN);
    new_node->port = port;
    new_node->sd = sd;
    new_node->next = (*head_ref);
    new_node->timestamp_login = (time_t*) malloc(sizeof(time_t));
    time(new_node->timestamp_login);
    //printf("TEST 1\n");
    new_node->timestamp_logout = NULL;
    //printf("TEST 2\n");
    (*head_ref) = new_node;
  }
  // ALTRIMENTI RESETTO I TIMESTAMP
  //reset_timestamp(head_ref, username);
  update_register(head_ref, username, port, sd);
}

// ****************************************
// Elimina il registro passato
// ****************************************
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

// ****************************************
// Elimina dal registro l'utente associato
// alla socket specificata.
// ****************************************
void delete_by_socket(struct user_data** head_ref, int sd){
  struct user_data* user = *head_ref, *prev;
  if(user != NULL && user->sd == sd) {
    *head_ref = user->next;
    free(user);
    return;
  }
  while(user != NULL && user->sd != sd) {
    prev = user;
    user = user->next;
  }
  if(user == NULL) return;
  prev->next = user->next;
  free(user);
}

// ****************************************
// ritorna lo username dell'utente associato
// alla socket specificata
// ****************************************
char* find_user_by_socket(struct user_data** head_ref, int sd){
  struct user_data* user = *head_ref;
  while(user!=NULL){
    if(user->sd == sd) return user->user_dest;
    user = user->next;
  }
  return NULL;
}

// ****************************************
// stampa la lista degli utenti online
// nel formato specificato:
// username*porta*timestam
// ****************************************
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

// ****************************************
// verifica che l'utente che le credenziali
// inserite corrispondano ad un account
// e nel caso, aggiunge l'utente al registro
// ****************************************
int login(struct user_data** head_ref, char *user, char *pw, short port, int sd){
  if(login_check(user, pw)){
    push_registro(head_ref, user, port, sd);
    return 1;
  }
  return 0;
}

// ****************************************
// setta il timestamp di logout dell'account
// che ha fatto richiesta
// ****************************************
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

// ****************************************
// presa la lista degli utenti
// che hanno messaggi pendenti
// ritornaa il riferimento alla lista dei
// messaggi pendenti dell'utente specificato
// ****************************************
struct hanging_msg** find_pending_msg(struct chat** head_ref, char* username){
  struct chat* c_chat = *head_ref;
  while(c_chat != NULL){
    if(strcmp(c_chat->name, username)==0) return &(c_chat->l_msg);
    c_chat = c_chat->next;
  }
  return append_dest(head_ref, username);
}

// ****************************************
// aggiunge alla lista dei destinatari
// un nuovo destinatario, in testa
// e ritorna il riferimento alla lista dei messaggi
// ****************************************
struct hanging_msg** append_dest(struct chat** head_ref, char* username){
  struct chat* new_node = (struct chat*) malloc(sizeof(struct chat));
  strncpy(new_node->name, username, S_BUF_LEN);
  new_node->l_msg = NULL;
  new_node->next = (*head_ref);
  (*head_ref) = new_node;
  return &(new_node->l_msg);
}

// ****************************************
// aggiunge un messaggio pendente alla
// lista specificata,
// l'inserimento è in coda
// ****************************************
void append_msg(struct hanging_msg** head_ref, char* dest_user, char* send_user, char* msg, int seq_n){
  if(*head_ref==NULL){
    struct hanging_msg* new_msg = (struct hanging_msg*) malloc(sizeof(struct hanging_msg));
    strncpy(new_msg->text, msg, BUF_LEN);
    strncpy(new_msg->send, send_user, S_BUF_LEN);
    strncpy(new_msg->dest, dest_user, S_BUF_LEN);
    new_msg->seq_n = seq_n;
    new_msg->timestamp = (time_t*)malloc(sizeof(time_t));
    time(new_msg->timestamp);
    new_msg->next = *head_ref;
    *head_ref = new_msg;
  }
  else append_msg(&(*head_ref)->next, dest_user, send_user, msg, seq_n);
}

// ****************************************
// funzione di debug: per stampare i messaggi pendenti
// e controllare lo stato del server
// ****************************************
void prind_all_hanging_msg(struct chat* head){
  struct chat* c_dest = head;
  struct hanging_msg* c_msg = NULL;
  while (c_dest!=NULL) {
    c_msg = c_dest->l_msg;
    printf("MESSAGGI PER %s\n", c_dest->name);
    while (c_msg!=NULL) {
      printf("%s : %s", c_msg->send, c_msg->text);
      c_msg = c_msg->next;
    }
    c_dest = c_dest->next;
  }
}

// ****************************************
// Inserisco lo username nella lista dei mittenti
// specificata
// ****************************************
void append_sender(struct sender** sender_head_ref, char* username){
  struct sender* new_sender = (struct sender*)malloc(sizeof(struct sender));
  strncpy(new_sender->username, username, S_BUF_LEN);
  new_sender->n_msg = 1;
  new_sender->next = *sender_head_ref;
  *sender_head_ref = new_sender;
}


// ****************************************
// Aggiungo 1 al numero dei messaggi ricevuti
// dal mittente, se il mittente non è nella lista,
// lo aggiungo (1 messaggio già inserito)
// ****************************************
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

// ****************************************
// data una lista di messaggi, creo una
// lista dei mittenti, ad ognuno è associato
// il numero di messaggi che ha spedito
// ****************************************
void find_sender(struct hanging_msg* msg_head, struct sender** sender_head_ref){
  struct hanging_msg* c_msg = msg_head;
  while(c_msg!=NULL){
    add_one_msg(sender_head_ref, c_msg->send);
    c_msg=c_msg->next;
  }
}

// ****************************************
// rimuove dalla lista il primo messaggio
// dell'utente specificato. se non ne trova
// ritorna NULL
// ****************************************
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

// ****************************************
// data la lista dei messaggi e uno username
// scrive all'indirizzo passato, il timestamp
// del messaggio più recente del mittente specificato.
// (l'ultimo della lista)
// ****************************************
void find_last_timestamp(time_t** timestamp, struct hanging_msg* l_msg, char* username){
  struct hanging_msg* c_msg = l_msg;
  while(c_msg!=NULL){
    if(strcmp(c_msg->send, username)==0) *timestamp = c_msg->timestamp;
    c_msg = c_msg->next;
  }
}

void display_help_message(){
  printf("Digita un comando: \n\n");
  printf("1) help --> mostra i dettagli dei comandi\n");
  printf("2) list --> mostra un elenco degli utenti connsessi\n");
  printf("3) esc  --> chiude il server\n");
}

// ****************************************
// la funzione forward_msg(...) inoltra il
// messaggio ricevuto dal mittente, alla porta
// passata, dove è in ascolto il destinatario
// ****************************************
int forward_msg(short port, char* sender, int seq_n, char* msg){
  int len, ret, cht_sd;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  struct sockaddr_in dest_addr;

  // apro la socket di destinazione
  cht_sd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);
  ret = connect(cht_sd, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
  if(ret<0){
    return 0;
  }
  //INVIO LA RICHIESTA DI MESSAGGIO
  sprintf(buffer,"%s", "MSG");
  ret = send_all(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL MITTENTE
  len = strlen(sender)+1;
  lmsg = htons(len);
  ret = send_all(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //INVIO IL MITTENTE
  sprintf(buffer,"%s", sender);
  ret = send_all(cht_sd, (void*) buffer, len, 0);

  //INVIO LA LUNGHEZZA DEL MESSAGGIO
  len = strlen(msg)+1;
  lmsg = htons(len);
  ret = send_all(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //INVIO IL MESSAGGIO
  ret = send_all(cht_sd, (void*) msg, len, 0);

  //INVIO IL NUMERO DI SEQUENZA
  lmsg = htons(seq_n);
  ret = send_all(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  // chiudo la socket di destinazione
  close(cht_sd);

  return 1;
}

// ****************************************
// la funzione forward_msg_ack inoltra l'ack
// di ricezione al mittente, che è in ascolto
// sulla porta specificata.
// l'ack è costituito da username destinatario
// e numero di sequenza...
// ****************************************
void forward_msg_ack(short port, char* dest, int seq_n){
  int len, ret, cln_sd;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  struct sockaddr_in dest_addr;

  //APRO LA SOCKET DI DESTINAZIONE
  cln_sd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);
  ret = connect(cln_sd, (struct sockaddr*)&dest_addr, sizeof(dest_addr));

  // FACCIO RICHIESTA DI MESSAGE ACK
  sprintf(buffer,"%s", "MAK");
  ret = send_all(cln_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DELLO USERNAME DESTINATARIO
  len = strlen(dest)+1;
  lmsg = htons(len);
  ret = send_all(cln_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //INVIO LO USERNAME DESTINATARIO
  sprintf(buffer,"%s", dest);
  ret = send_all(cln_sd, (void*) buffer, len, 0);

  //INVIO IL NUMERO DI SEQUENZA
  lmsg = htons(seq_n);
  ret = send_all(cln_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //CHIUDO LA SOCKET
  close(cln_sd);
}
