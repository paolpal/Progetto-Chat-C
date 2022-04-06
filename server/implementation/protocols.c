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
#include "../constants.h"
#include "../networking.h"
#include "../utility.h"
//#include "protocols.h"

void signup_protocol(int i, char* buffer){
  //printf("Protocollo di iscrizione avviato\n");
  char *username, *password;
  uint16_t lmsg;
  int len;
  //printf("Ricevo lunghezza username\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  username = (char*) malloc(len*sizeof(char));
  //printf("Ricevo username\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);
  //printf("%s\n", username);
  //printf("Ricevo lunghezza password\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  password = (char*) malloc(len*sizeof(char));
  //printf("Ricevo password\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", password);
  //printf("%s\n", password);
  if(signup(username, password)){
    sprintf(buffer, "%s", "SIGNED");
  }
  else{
    sprintf(buffer, "%s", "FAILED");
  }
  send_all(i, (void*) buffer, ACK_LEN, 0);
  free(username);
  free(password);
  return;
}

void login_protocol(int i, struct user_data** utenti, char* buffer){
  //printf("Protocollo di login avviato\n");
  char *username, *password;
  uint16_t lmsg;
  int len;
  short port;

  //printf("Ricevo lunghezza username\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  username = (char*) malloc(len*sizeof(char));
  //printf("Ricevo username\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);
  //printf("%s\n", username);
  //printf("Ricevo lunghezza password\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  password = (char*) malloc(len*sizeof(char));
  //printf("Ricevo password\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", password);
  //printf("%s\n", password);
  //printf("Ricevo porta\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  port = ntohs(lmsg);
  //printf("Porta ricevuta %d\n", port);

  if(login(utenti, username, password, port)){
    sprintf(buffer, "%s", "LOGGED");
  }
  else{
    sprintf(buffer, "%s", "FAILED");
  }
  send_all(i, (void*) buffer, ACK_LEN, 0);
  free(username);
  free(password);
  return;
}

void logout_protocol(int i, struct user_data** utenti, char* buffer){
  //printf("Protocollo di login avviato\n");
  char *username;
  uint16_t lmsg;
  int len;

  //printf("Ricevo lunghezza username\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  username = (char*) malloc(len*sizeof(char));
  //printf("Ricevo username\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);
  //printf("%s\n", username);

  if(logout(utenti, username)){
    sprintf(buffer, "%s", "EXITED");
  }
  else{
    sprintf(buffer, "%s", "FAILED");
  }
  send_all(i, (void*) buffer, ACK_LEN, 0);
  free(username);
  return;
}

void new_chat_protocol(int i, struct user_data** utenti, struct destinatario** destinatari,  char* buffer){
  char *dest, *send, *msg;
  uint16_t lmsg;
  int len;
  short port;

  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));

  //RICEVO IL NOME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  //RICEVO LA LUNGHEZZA DEL NOME MITTENTE
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  send = (char*) malloc(len*sizeof(char));

  //RICEVO IL NOME MITTENTE
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", send);

  //RICEVO LA LUNGHEZZA DEL MESSAGGIO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //printf("Lunghezza messaggio : %d\n", len);
  msg = (char*) malloc(len*sizeof(char));

  //RICEVO IL MESSAGGIO
  recv_all(i, (void*)buffer, len, 0);
  //sscanf(buffer, "%s", msg);
  strcpy(msg,buffer);

  //printf("%s\n", msg);

  // ricerco il destinatario tra gli utenti attivi
  // anche timestamp_logout deve essere NULL
  port = find_port(utenti, dest);
  //lo trovo : inoltro
  int rep = 0;
  int forwarded = 0;
  if(port!=0) while(!forwarded && rep < 3) {
    forwarded = forward_msg(port, send, msg);
    rep++;
  }
  //NON lo trovo : appendo
  else {
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    // APPENDERE IL MESSAGGIO PENDENTE
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    struct hanging_msg** msg_list_ref;
    msg_list_ref = find_pending_msg(destinatari, dest);
    append_msg(msg_list_ref, dest, send, msg);
  }

  //rispondo sempre con la porta: se non l'ho trovata contiene ZERO
  lmsg = htons(port);
  send_all(i,(void*) &lmsg, sizeof(uint16_t), 0);
}

int forward_msg(short port, char* sender, char* msg){
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
  //printf("Invio richiesta di CHAT\n");
  ret = send(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL MITTENTE
  len = strlen(sender)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //INVIO IL MITTENTE
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", sender);
  ret = send(cht_sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //INVIO LA LUNGHEZZA DEL MESSAGGIO
  len = strlen(msg)+1;
  //printf("Messaggio lungo : %d\n", len);
  lmsg = htons(len);
  //printf("LUNGHEZZA messaggio : %d (%d)\n", len, (int)lmsg);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //INVIO IL MESSAGGIO
  //printf("Invio il messaggio : %s\n", msg);
  ret = send(cht_sd, (void*) msg, len, 0);

  // chiudo la socket di destinazione
  close(cht_sd);

  return 1;
}

void hanging_protocol(int i, struct destinatario** destinatari, char* buffer){
  char *dest;
  uint16_t lmsg;
  int len, ret;

  struct sender* l_sender = NULL;
  struct sender* c_sender = NULL;
  struct hanging_msg** l_msg_ref;

  //printf("INIZIO PROCEDURA DI HANGING\n");

  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO (utente che fa richiesta)
  //printf("Ricevo la lunghezza del nome\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //printf("Ricevo la lunghezza del nome\n");

  //RICEVO IL NOME DESTINATARIO
  //printf("Ricevo la lunghezza del nome\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);
  //printf("%s\n", dest);
  //fflush(stdout);

  l_msg_ref = find_pending_msg(destinatari, dest);
  find_sender(*l_msg_ref, &l_sender);

  c_sender = l_sender;
  while(c_sender!=NULL){
    //INVIO LA LUNGHEZZA DEL MITTENTE
    len = strlen(c_sender->username)+1;
    lmsg = htons(len);
    //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    //INVIO IL MITTENTE
    //printf("Invio lo username\n");
    sprintf(buffer,"%s", c_sender->username);
    ret = send_all(i, (void*) buffer, len, 0);

    //INVIO LA IL NUMERO DI MESSAGGI DEL MITTENTE
    lmsg = htons(c_sender->n_msg);
    //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    c_sender = c_sender->next;
  }

  // INVIO ZERO : FINE DELLA TRASMISSIONE
  lmsg = htons(0);
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);


}

void show_protocol(int i, struct destinatario** destinatari, char* buffer){
  char *dest;
  char *sender;
  uint16_t lmsg;
  int len, ret;

  struct hanging_msg** l_msg_ref;
  struct hanging_msg* msg;

  printf("INIZIO PROCEDURA DI SHOW\n");

  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO (utente che fa richiesta)
  //printf("Ricevo la lunghezza del nome\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //printf("Ricevo la lunghezza del nome\n");

  //RICEVO IL NOME DESTINATARIO
  //printf("Ricevo la lunghezza del nome\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);
  //printf("%s\n", dest);
  //fflush(stdout);

  //RICEVO LA LUNGHEZZA DEL NOME MITTENTE
  //printf("Ricevo la lunghezza del nome\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  sender = (char*) malloc(len*sizeof(char));
  //printf("Ricevo la lunghezza del nome\n");

  //RICEVO IL NOME MITTENTE
  //printf("Ricevo la lunghezza del nome\n");
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", sender);
  //printf("%s\n", dest);
  //fflush(stdout);

  l_msg_ref = find_pending_msg(destinatari, dest);

  while((msg = remove_msg(l_msg_ref, sender))!=NULL){
    //INVIO LA LUNGHEZZA DEL MESSAGGIO
    len = strlen(msg->msg)+1;
    lmsg = htons(len);
    //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    //INVIO IL MESSAGGIO
    //printf("Invio lo username\n");
    sprintf(buffer,"%s", msg->msg);
    ret = send_all(i, (void*) buffer, len, 0);

    //free(msg);
  }

  //TERMINO LA PROCEDURA INVIANDO ZERO
  lmsg = htons(0);
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}

void group_protocol(int i, struct user_data** utenti, char* buffer){
  uint16_t lmsg;
  int len, ret;

  struct user_data* c_user = *utenti;

  while(c_user!=NULL){
    if(c_user->timestamp_logout==NULL){
      //INVIO LA LUNGHEZZA DELLO USERNAME
      len = strlen(c_user->user_dest)+1;
      lmsg = htons(len);
      ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

      //INVIO LO USERNAME
      sprintf(buffer,"%s", c_user->user_dest);
      ret = send_all(i, (void*) buffer, len, 0);
    }
    c_user = c_user->next;
  }
  //TERMINO LA PROCEDURA INVIANDO ZERO
  lmsg = htons(0);
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}
