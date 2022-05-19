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
#include "../protocols.h"

// ************************************
// il protocollo di iscrizione riceve
// uno username e una password e verifica
// che lo username sia disponibile
// quindi lo aggiunge al file di configurazione
// ************************************
void signup_protocol(int i, char* buffer){
  char username[S_BUF_LEN], password[S_BUF_LEN];
  uint16_t lmsg;
  int len;

  //RICEVO LA LUNGHEZZA DELLO USERNAME
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //username = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  //RICEVO LA LUNGHEZZA DELLA PASSWORD
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //password = (char*) malloc(len*sizeof(char));
  //RICEVO LA PASSWORD
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", password);

  if(signup(username, password)){
    sprintf(buffer, "%s", "SIGNED");
  }
  else{
    sprintf(buffer, "%s", "FAILED");
  }
  send_all(i, (void*) buffer, ACK_LEN, 0);
  //free(username);
  //free(password);
  return;
}

// ************************************
// Il protocollo di login attende uno
// username, una password e una porta
// quindi controlla se username e password
// corrispondono.
// Risponde con la valutazione.
// se il login avviene, le informazioni
// sono memorizzate nel registro
// ************************************
void login_protocol(int i, struct user_data** utenti, struct chat** l_char_r, char* buffer){
  char username[S_BUF_LEN], password[S_BUF_LEN];
  uint16_t lmsg;
  int len;
  short port;
  int logged=0;
  struct msg_ack** l_ack_r;
  struct msg_ack* ack;
  //RICEVO LA LUNGHEZZA DELLO USERNAME
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //username = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  //RICEVO LA LUNGHEZZA DELLA PASSWORD
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //password = (char*) malloc(len*sizeof(char));
  //RICEVO LA PASSWORD
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", password);

  //RICEVO LA PORTA DI ASCOLTO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  port = ntohs(lmsg);

  logged = login(utenti, username, password, port, i);
  if(!logged){
    sprintf(buffer, "%s", "FAILED");
    send_all(i, (void*) buffer, ACK_LEN, 0);
    return;
  }
  else{
    sprintf(buffer, "%s", "LOGGED");
    send_all(i, (void*) buffer, ACK_LEN, 0);
  }

  printf("Cerco la chat...\n");
  l_ack_r = &(find_chat(l_char_r, username)->l_ack);
  printf("Inoltro gli ACK...\n");
  while((ack=remove_ack(l_ack_r))!=NULL){
    forward_msg_ack(port, ack->dest, ack->seq_n);
  }

  return;
}

// ****************************************
// il protocollo di logout riceve uno username
// quindi aggiorna i dati del registro corrispondenti
// risponde in base all'esito
// ****************************************
void logout_protocol(int i, struct user_data** l_user_r, char* buffer){
  char username[S_BUF_LEN];
  uint16_t lmsg;
  int len;

  //RICEVO LA LUNGHEZZA DELLO USERNAME
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //username = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  if(logout(l_user_r, username)){
    sprintf(buffer, "%s", "EXITED");
  }
  else{
    sprintf(buffer, "%s", "FAILED");
  }
  send_all(i, (void*) buffer, ACK_LEN, 0);
  //free(username);
  return;
}

// ****************************************
// il protocollo di new chat riceve un
// messaggio da un client e prova ad inoltrarlo
// ad un'altro.
// restituisce al primo la pora su cui
// contattare direttamente il destinatario.
// Se il messaggio è privo di numero sequenziale,
// ne assegna uno randomico e lo restituisce
// al mittente.
// ****************************************
void new_chat_protocol(int i, struct user_data** utenti, struct chat** destinatari,  char* buffer){
  char *dest, *send, *msg;
  uint16_t lmsg;
  int len, seq_n;
  short port;

  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  printf("<LOG> Ricevo il nome destinatario: %s\n", dest);

  //RICEVO LA LUNGHEZZA DEL NOME MITTENTE
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  send = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME MITTENTE
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", send);

  printf("<LOG> Ricevo il nome mittente: %s\n", send);

  //RICEVO LA LUNGHEZZA DEL MESSAGGIO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  msg = (char*) malloc(len*sizeof(char));
  //RICEVO IL MESSAGGIO
  recv_all(i, (void*)buffer, len, 0);
  strcpy(msg, buffer);

  printf("<LOG> Ricevo il messaggio\n");

  //RICEVO IL NUMERO DI SEQUENZA
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  seq_n = ntohs(lmsg);

  printf("<LOG> Cerco il destinatario\n");

  // ricerco il destinatario tra gli utenti online
  port = find_port(utenti, dest);
  printf("<LOG> Fine ricerca\n");
  //lo trovo : inoltro
  int rep = 0;
  int forwarded = 0;
  if(port!=0) while(!forwarded && rep < 3) {
    printf("<LOG> Inoltro #%d a %d\n", rep, port);
    forwarded = forward_msg(port, send, seq_n, msg);
    rep++;
  }
  //NON lo trovo : appendo
  else {
    printf("<LOG> Appendo\n");
    struct hanging_msg** msg_list_ref;
    msg_list_ref = find_hanging_msg(destinatari, dest);
    append_msg(msg_list_ref, dest, send, msg, seq_n);
  }

  //rispondo sempre con la porta: se non l'ho trovata contiene ZERO
  lmsg = htons(port);
  send_all(i,(void*) &lmsg, sizeof(uint16_t), 0);
  printf("<LOG> Rispondo con l'esito\n");
}

// ********************************************
// il protocollo di hanging riceve un nome destinatario
// e restituisce l'elenco di utenti che hanno contattato
// con numero di messaggi e timestamp del messaggio più recente
// ********************************************
void hanging_protocol(int i, struct chat** destinatari, char* buffer){
  char *dest;
  uint16_t lmsg;
  int len, ret;
  time_t timestamp = 0;

  struct sender* l_sender = NULL;
  struct sender* c_sender = NULL;
  struct hanging_msg** l_msg_ref;

  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO (utente che fa richiesta)
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  // cerco la lista di messsaggi associata al destinatario
  l_msg_ref = find_hanging_msg(destinatari, dest);
  //raccolgo tutti i mittenti possibili, dalla lista
  find_sender(*l_msg_ref, &l_sender);

  // per ogni mittente raccolgo i dati associati e li mando al client
  c_sender = l_sender;
  while(c_sender!=NULL){
    //INVIO LA LUNGHEZZA DEL MITTENTE
    len = strlen(c_sender->username)+1;
    lmsg = htons(len);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
    //INVIO IL MITTENTE
    sprintf(buffer,"%s", c_sender->username);
    ret = send_all(i, (void*) buffer, len, 0);

    //INVIO IL NUMERO DI MESSAGGI DEL MITTENTE
    lmsg = htons(c_sender->n_msg);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    //CERCA IL TIMESTAMP PIU' RECENTE
    printf("CERCO IL TIMESTAMP PIU' RECENTE");
    find_last_timestamp(&timestamp, *l_msg_ref, c_sender->username);
    sprintf(buffer,"%s", ctime(&timestamp));
    //INVIO LA LUNGHEZZA DEL TIMESTAMP
    len = strlen(buffer)+1;
    lmsg = htons(len);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
    // INVIO IL TIMESTAMP
    ret = send_all(i, (void*) buffer, len, 0);

    c_sender = c_sender->next;
  }
  // INVIO ZERO : FINE DELLA TRASMISSIONE
  lmsg = htons(0);
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}

// ****************************************
// il protocollo riceve il nome del destinatario
// e del mittente
// quindi manda i messaggi del mittente al
// destinatario in ordine di ricezzione
// ****************************************
void show_protocol(int i, struct chat** destinatari, char* buffer){
  char *dest;
  char *sender;
  uint16_t lmsg;
  int len, ret;

  struct hanging_msg** l_msg_ref;
  struct hanging_msg* msg;

  printf("INIZIO PROCEDURA DI SHOW\n");

  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO (utente che fa richiesta)
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  //RICEVO LA LUNGHEZZA DEL NOME MITTENTE
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  sender = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME MITTENTE
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", sender);

  // cerco la lista di messaggi pendenti per il destinatario
  l_msg_ref = &(find_chat(destinatari, dest)->l_msg);

  // rimuovo il primo messaggio del mittente dalla lista in ciclo
  // finche non ne resta nessuno
  // termino quando ritorno NULL
  while((msg = remove_msg(l_msg_ref, sender))!=NULL){
    //INVIO LA LUNGHEZZA DEL MESSAGGIO
    len = strlen(msg->text)+1;
    lmsg = htons(len);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
    //INVIO IL MESSAGGIO
    sprintf(buffer,"%s", msg->text);
    ret = send_all(i, (void*) buffer, len, 0);
    //INVIO IL NUMERO SEQUENZIALE
    lmsg = htons(msg->seq_n);
    ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    //free(msg);
  }

  //TERMINO LA PROCEDURA INVIANDO ZERO
  lmsg = htons(0);
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}

// ************************************
// il protocollo di group manda all'utente
// che ha fatto richiesta la lista degli
// utenti online
// ************************************
void group_protocol(int i, struct user_data** utenti, char* buffer){
  uint16_t lmsg;
  int len, ret;

  struct user_data* c_user = *utenti;

  while(c_user!=NULL){
    if(c_user->t_logout==0){
      //INVIO LA LUNGHEZZA DELLO USERNAME
      len = strlen(c_user->username)+1;
      lmsg = htons(len);
      ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

      //INVIO LO USERNAME
      sprintf(buffer,"%s", c_user->username);
      ret = send_all(i, (void*) buffer, len, 0);
    }
    c_user = c_user->next;
  }
  //TERMINO LA PROCEDURA INVIANDO ZERO
  lmsg = htons(0);
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}

// ****************************************
// il protocollo di forward degli ACK
// di ricezzione è necessario
// in quanto il server si occupa di
// instradare correttamente gli ACK
// ****************************************
void forw_msg_ack_protocol(int i, struct user_data** utenti, struct chat** l_chat_r, char* buffer){
  int len, ret, seq_n;
  short port;
  uint16_t lmsg;
  char *sender, *dest;
  struct chat* send_chat;
  struct msg_ack* ack;

  //RICEVO LA LUNGHEZZA DELLO USERNAME MITTENTE
  ret = recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  sender = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME MITTENTE
  ret = recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", sender);

  //RICEVO LA LUNGHEZZA DELLO USERNAME DESTINATARIO
  ret = recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME DESTINATARIO
  ret = recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  //RICEVO IL NUMERO DI SEQUENZA
  ret = recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  seq_n = ntohs(lmsg);

  port = find_port(utenti, sender);
  if(port != 0) forward_msg_ack(port, dest, seq_n);
  else{
    send_chat = find_chat(l_chat_r, sender);
    ack = create_ack(dest, seq_n);
    append_ack(&(send_chat->l_ack), ack);
  }
}

// ****************************************
// il protocollo di check online controlla
// che l'utente specificato sia online
// il controllo è necessario per assicurarsi
// che solo utenti online siano aggiunti ai gruppi
// ****************************************
void online_check_protocol(int i, struct user_data** utenti, char* buffer){
  int len, ret;
  uint16_t lmsg;
  char *user;

  //RICEVO LA LUNGHEZZA DELLO USERNAME
  ret = recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  user = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  ret = recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", user);

  //VALUTO SE L'UTENTE E' ONLINE
  if(is_online(utenti, user)) ret = 1;
  else ret = 0;
  lmsg = htons(ret);
  //INVIO LA VALUTAZIONE
  ret = send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}
