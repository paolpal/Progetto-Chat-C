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
  fprintf(stderr,"<LOG> Ricevo lo USERNAME\n");
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //username = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  fprintf(stderr,"<LOG> Ricevo la PASSWORD\n");
  //RICEVO LA LUNGHEZZA DELLA PASSWORD
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //password = (char*) malloc(len*sizeof(char));
  //RICEVO LA PASSWORD
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", password);

  fprintf(stderr,"<LOG> Valuto l'ISCRIZIONE\n");
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

  fprintf(stderr,"<LOG> Ricevo lo USERNAME\n");
  //RICEVO LA LUNGHEZZA DELLO USERNAME
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //username = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  fprintf(stderr,"<LOG> Ricevo la PASSWORD\n");
  //RICEVO LA LUNGHEZZA DELLA PASSWORD
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  //password = (char*) malloc(len*sizeof(char));
  //RICEVO LA PASSWORD
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", password);

  fprintf(stderr,"<LOG> Ricevo la PORTA\n");
  //RICEVO LA PORTA DI ASCOLTO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  port = ntohs(lmsg);

  fprintf(stderr,"<LOG> Valuto l'ACCESSO\n");
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

  fprintf(stderr,"<LOG> Cerco la chat...\n");
  l_ack_r = &(find_chat(l_char_r, username)->l_ack);
  fprintf(stderr,"<LOG> Inoltro gli ACK...\n");
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

  printf("Eseguo il LOGOUT\n");
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

  fprintf(stderr,"<LOG> Ricevo lo USERNAME destinatario\n");
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

  fprintf(stderr,"<LOG> Ricevo lo USERNAME mittente\n");

  //RICEVO LA LUNGHEZZA DEL MESSAGGIO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  msg = (char*) malloc(len*sizeof(char));
  //RICEVO IL MESSAGGIO
  recv_all(i, (void*)buffer, len, 0);
  strcpy(msg, buffer);

  fprintf(stderr,"<LOG> Ricevo il MESSAGGIO\n");

  //RICEVO IL NUMERO DI SEQUENZA
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  seq_n = ntohs(lmsg);

  fprintf(stderr,"<LOG> Cerco il DESTINATARIO\n");

  // ricerco il destinatario tra gli utenti online
  port = find_port(utenti, dest);
  fprintf(stderr,"<LOG> Fine ricerca...\n");
  //lo trovo : inoltro
  int rep = 0;
  int forwarded = 0;
  if(port!=0) while(!forwarded && rep < 3) {
    fprintf(stderr,"<LOG> Inoltro...\n", rep, port);
    forwarded = forward_msg(port, send, seq_n, msg);
    rep++;
  }
  //NON lo trovo : appendo
  else {
    fprintf(stderr,"<LOG> Appendo... \n");
    struct hanging_msg** msg_list_ref;
    msg_list_ref = find_hanging_msg(destinatari, dest);
    append_msg(msg_list_ref, dest, send, msg, seq_n);
  }

  //rispondo sempre con la porta: se non l'ho trovata contiene ZERO
  lmsg = htons(port);
  send_all(i,(void*) &lmsg, sizeof(uint16_t), 0);
  fprintf(stderr,"<LOG> Rispondo con l'esito...\n");
}

// ********************************************
// il protocollo di hanging riceve un nome destinatario
// e restituisce l'elenco di utenti che hanno contattato
// con numero di messaggi e timestamp del messaggio più recente
// ********************************************
void hanging_protocol(int i, struct chat** destinatari, char* buffer){
  char *dest;
  uint16_t lmsg;
  int len;
  time_t timestamp = 0;

  struct sender* l_sender = NULL;
  struct sender* c_sender = NULL;
  struct hanging_msg** l_msg_ref;

  fprintf(stderr,"<LOG> Ricevo lo USERNAME destinatario\n");
  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO (utente che fa richiesta)
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  fprintf(stderr,"<LOG> Cerco la lista dei messsaggi del DESTINATARIO\n");
  // cerco la lista di messsaggi associata al destinatario
  l_msg_ref = find_hanging_msg(destinatari, dest);
  //raccolgo tutti i mittenti possibili, dalla lista
  fprintf(stderr,"<LOG> Creo la lista dei MITTENTI\n");
  find_sender(*l_msg_ref, &l_sender);

  // per ogni mittente raccolgo i dati associati e li mando al client
  fprintf(stderr,"<LOG> Per ogni MITTENTE raccolgo i dati...\n");
  c_sender = l_sender;
  while(c_sender!=NULL){
    fprintf(stderr,"<LOG> Invio lo USERNAME mittente\n");
    //INVIO LA LUNGHEZZA DEL MITTENTE
    len = strlen(c_sender->username)+1;
    lmsg = htons(len);
    send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
    //INVIO IL MITTENTE
    sprintf(buffer,"%s", c_sender->username);
    send_all(i, (void*) buffer, len, 0);

    fprintf(stderr,"<LOG> Invio il NUMERO di messaggi \n");
    //INVIO IL NUMERO DI MESSAGGI DEL MITTENTE
    lmsg = htons(c_sender->n_msg);
    send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    //CERCA IL TIMESTAMP PIU' RECENTE
    fprintf(stderr,"<LOG> Cerco il timestamp più recente...\n");
    find_last_timestamp(&timestamp, *l_msg_ref, c_sender->username);
    sprintf(buffer,"%s", ctime(&timestamp));
    fprintf(stderr,"<LOG> Invio il TIMESTAMP\n");
    //INVIO LA LUNGHEZZA DEL TIMESTAMP
    len = strlen(buffer)+1;
    lmsg = htons(len);
    send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
    // INVIO IL TIMESTAMP
    send_all(i, (void*) buffer, len, 0);

    c_sender = c_sender->next;
  }
  // INVIO ZERO : FINE DELLA TRASMISSIONE
  lmsg = htons(0);
  send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
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
  int len;

  struct hanging_msg** l_msg_ref;
  struct hanging_msg* msg;

  fprintf(stderr,"<LOG> Ricevo lo USERNAME destinatario\n");
  //RICEVO LA LUNGHEZZA DEL NOME DESTINATARIO (utente che fa richiesta)
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  fprintf(stderr,"<LOG> Ricevo lo USERNAME mittente\n");
  //RICEVO LA LUNGHEZZA DEL NOME MITTENTE
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  sender = (char*) malloc(len*sizeof(char));
  //RICEVO IL NOME MITTENTE
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", sender);

  fprintf(stderr,"<LOG> Cerco la lista dei messsaggi del DESTINATARIO\n");
  // cerco la lista di messaggi pendenti per il destinatario
  l_msg_ref = &(find_chat(destinatari, dest)->l_msg);

  // rimuovo il primo messaggio del mittente dalla lista in ciclo
  // finche non ne resta nessuno
  // termino quando ritorno NULL
  fprintf(stderr,"<LOG> Invio i messaggi del mittente specificato...\n");
  while((msg = remove_msg(l_msg_ref, sender))!=NULL){
    //INVIO LA LUNGHEZZA DEL MESSAGGIO
    len = strlen(msg->text)+1;
    lmsg = htons(len);
    send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
    //INVIO IL MESSAGGIO
    sprintf(buffer,"%s", msg->text);
    send_all(i, (void*) buffer, len, 0);
    //INVIO IL NUMERO SEQUENZIALE
    lmsg = htons(msg->seq_n);
    send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

    //free(msg);
  }

  //TERMINO LA PROCEDURA INVIANDO ZERO
  lmsg = htons(0);
  send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}

// ************************************
// il protocollo di group manda all'utente
// che ha fatto richiesta la lista degli
// utenti online
// ************************************
void group_protocol(int i, struct user_data** utenti, char* buffer){
  uint16_t lmsg;
  int len;

  struct user_data* c_user = *utenti;

  while(c_user!=NULL){
    if(c_user->t_logout==0){
      //INVIO LA LUNGHEZZA DELLO USERNAME
      len = strlen(c_user->username)+1;
      lmsg = htons(len);
      send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);

      //INVIO LO USERNAME
      sprintf(buffer,"%s", c_user->username);
      send_all(i, (void*) buffer, len, 0);
    }
    c_user = c_user->next;
  }
  //TERMINO LA PROCEDURA INVIANDO ZERO
  lmsg = htons(0);
  send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}

// ****************************************
// il protocollo di forward degli ACK
// di ricezzione è necessario
// in quanto il server si occupa di
// instradare correttamente gli ACK
// ****************************************
void forw_msg_ack_protocol(int i, struct user_data** utenti, struct chat** l_chat_r, char* buffer){
  int len, seq_n;
  short port;
  uint16_t lmsg;
  char *sender, *dest;
  struct chat* send_chat;
  struct msg_ack* ack;

  fprintf(stderr,"<LOG> Ricevo lo USERNAME mittente\n");
  //RICEVO LA LUNGHEZZA DELLO USERNAME MITTENTE
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  sender = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME MITTENTE
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", sender);

  fprintf(stderr,"<LOG> Ricevo lo USERNAME destinatario\n");
  //RICEVO LA LUNGHEZZA DELLO USERNAME DESTINATARIO
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  dest = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME DESTINATARIO
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", dest);

  fprintf(stderr,"<LOG> Ricevo il NUMERO di sequenza\n");
  //RICEVO IL NUMERO DI SEQUENZA
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  seq_n = ntohs(lmsg);

  fprintf(stderr,"<LOG> Cerco la PORTA di ascolto del mittente\n");
  port = find_port(utenti, sender);
  fprintf(stderr,"<LOG> Inoltro l'ACK...\n");
  if(port != 0) forward_msg_ack(port, dest, seq_n);
  else{
    fprintf(stderr,"<LOG> ACK appeso...\n");
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

  fprintf(stderr,"<LOG> Ricevo uno USERNAME...\n");
  //RICEVO LA LUNGHEZZA DELLO USERNAME
  recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  user = (char*) malloc(len*sizeof(char));
  //RICEVO LO USERNAME
  recv_all(i, (void*)buffer, len, 0);
  sscanf(buffer, "%s", user);

  fprintf(stderr,"<LOG> Valuto se l'UTENTE è ONLINE...\n");
  //VALUTO SE L'UTENTE E' ONLINE
  if(is_online(utenti, user)) ret = 1;
  else ret = 0;
  lmsg = htons(ret);
  //INVIO LA VALUTAZIONE
  send_all(i, (void*) &lmsg, sizeof(uint16_t), 0);
}
