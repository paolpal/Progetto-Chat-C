#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../protocols.h"
#include "../chatting.h"
#include "../constants.h"
#include "../networking.h"
#include "../filetransfer.h"

int signup_protocol_client(int sd, char* user, char* pw){
  int ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  // FACCIO RICHIESTA DI ISCRIZIONE
  sprintf(buffer,"%s", "SGN\0");
  //printf("Invio richiesta di ISCRZIONE\n");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)REQ_LEN-ret,ret);


  //INVIO LA LUNGHEZZA DEL NOME
  int len = strlen(user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO IL NOME
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", user);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);


  //INVIO LA LUNGHEZZA DELLA password
  len = strlen(pw)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA password :%d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO LA PASSWORD
  //printf("Invio la password\n");
  sprintf(buffer,"%s", pw);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //printf("%s\n", buffer);

  ret = recv_all(sd, (void*) buffer, ACK_LEN, 0);

  if(strcmp(buffer,"SIGNED")==0)return 1;
  else return 0;
}

int login_protocol_client(int sd, char* user, char* pw, short port){
  int ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  // FACCIO RICHIESTA DI LOGIN
  sprintf(buffer,"%s", "LIN");
  //printf("Invio richiesta di LOGIN\n");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)REQ_LEN-ret,ret);


  //INVIO LA LUNGHEZZA DEL NOME
  int len = strlen(user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO IL NOME
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", user);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);


  //INVIO LA LUNGHEZZA DELLA PASSWORD
  len = strlen(pw)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA password :%d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO LA PASSWORD
  //printf("Invio la password\n");
  sprintf(buffer,"%s", pw);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //INVIO PORTA
  lmsg = htons(port);
  //printf("Invio la porta\n");
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //printf("%s\n", buffer);

  ret = recv_all(sd, (void*) buffer, ACK_LEN, 0);

  if(strcmp(buffer,"LOGGED")==0)return 1;
  else return 0;
}

int logout_protocol_client(int sd, char* user){
  int ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  // FACCIO RICHIESTA DI LOGOUT
  sprintf(buffer,"%s", "OUT");
  //printf("Invio richiesta di LOGOUT\n");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)REQ_LEN-ret,ret);


  //INVIO LA LUNGHEZZA DEL NOME
  int len = strlen(user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO IL NOME
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", user);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  ret = recv_all(sd, (void*) buffer, ACK_LEN, 0);

  if(strcmp(buffer,"EXITED")==0)return 1;
  else return 0;
}

/*
Ritorna la socket stabilita con il destinatario, o 0 se l'invio è fallito
*/
//int new_chat_protocol_client(int sd, char* my_user, struct contatto *dest, char* msg){
int new_chat_protocol_client(int sd, char* my_user, char* dest_user, struct sockaddr_in* dest_addr, char* msg){
  int ret, len, cht_sd;
  uint16_t lmsg;
  short port;
  char buffer[BUF_LEN];

  sprintf(buffer,"%s", "CHT");
  //printf("Invio richiesta di CHAT\n");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL NOME DESTINATARIO
  len = strlen(dest_user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //INVIO IL NOME DESTINATARIO
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", dest_user);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //INVIO LA LUNGHEZZA DEL NOME MITTENTE
  len = strlen(my_user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //INVIO IL NOME MITTENTE
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", my_user);
  ret = send_all(sd, (void*) buffer, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //INVIO LA LUNGHEZZA DEL MESSAGGIO
  len = strlen(msg)+1;
  //printf("Messaggio lungo : %d\n", len);
  lmsg = htons(len);
  //printf("LUNGHEZZA messaggio : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  //printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //INVIO IL MESSAGGIO
  //printf("Invio lo messaggio\n");
  //printf("Invio il messaggio : %s", msg);
  ret = send_all(sd, (void*) msg, len, 0);
  //printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //RICEVO LA PORTA DEL CONTATTO
  ret = recv_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  port = ntohs(lmsg);

  if(port==0) return 0;

  cht_sd = socket(AF_INET, SOCK_STREAM, 0);

  memset(dest_addr, 0, sizeof(*dest_addr));
  dest_addr->sin_family = AF_INET;
  dest_addr->sin_port = htons(port);
  inet_pton(AF_INET, "127.0.0.1", &dest_addr->sin_addr);

  ret = connect(cht_sd, (struct sockaddr*)dest_addr, sizeof(*dest_addr));
  if(ret<0){
    return 0;
  }
  return cht_sd;
}

void hanging_protocol_client(int sd, char* user){
  int ret, len, n_msg;
  uint16_t lmsg;
  char buffer[BUF_LEN];
  char *sender;

  //INVIO LA RICHIESTA DI HANGING
  sprintf(buffer,"%s", "HNG");
  //printf("Invio richiesta di HANGING\n");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL DESTINATARIO (current client)
  len = strlen(user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL DESTINATARIO
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", user);
  ret = send_all(sd, (void*) buffer, len, 0);

  while(1){
    //RICEVO LA LUNGHEZZA DEL NOME MITTENTE
    recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg);
    if(len == 0) break;
    sender = (char*) malloc(len*sizeof(char));

    //RICEVO IL NOME MITTENTE
    recv_all(sd, (void*)buffer, len, 0);
    sscanf(buffer, "%s", sender);

    //RICEVO IL NUMERO DI MESSAGGI
    recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    n_msg = ntohs(lmsg);


    // *********************
    // RICEVI IL TIMESTAMP
    // *********************

    // RICEVO LA LUNGHEZZA DEL TIMESTAMP
    recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg);
    
    //RICEVO IL TIMESTAMP
    recv_all(sd, (void*)buffer, len, 0);

    printf("%s %d %s", sender, n_msg, buffer);
  }
}

void show_protocol_client(int sd, char* my_user, char* sender_user, struct chat** l_chat){
  int ret, len;
  uint16_t lmsg;
  char buffer[BUF_LEN];
  char *msg_text;
  struct msg* msg;

  //INVIO LA RICHIESTA DI HANGING
  sprintf(buffer,"%s", "SHW");
  //printf("Invio richiesta di HANGING\n");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL DESTINATARIO (current client)
  len = strlen(my_user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL DESTINATARIO
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", my_user);
  ret = send_all(sd, (void*) buffer, len, 0);

  //INVIO LA LUNGHEZZA DEL MITTENTE
  len = strlen(sender_user)+1;
  lmsg = htons(len);
  //printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL MITTENTE
  //printf("Invio lo username\n");
  sprintf(buffer,"%s", sender_user);
  ret = send_all(sd, (void*) buffer, len, 0);

  while(1){
    //RICEVO LA LUNGHEZZA DEL MESSAGGIO
    recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg);
    if(len == 0) break;
    msg_text = (char*) malloc(len*sizeof(char));

    //RICEVO IL MESSAGGIO
    recv_all(sd, (void*)buffer, len, 0);
    strcpy(msg_text,buffer);

    msg = (struct msg*) malloc(sizeof(struct msg));
    msg->dest = NULL;
    len = strlen(sender_user)+1;
    msg->sender = (char*) malloc(len*sizeof(char));
    strcpy(msg->sender, sender_user);
    len = strlen(msg_text)+1;
    msg->text = (char*) malloc(len*sizeof(char));
    strcpy(msg->text, msg_text);
    msg->next = NULL;

    accoda_messaggio(l_chat, msg);
    stampa_messaggio(msg);
  }
}

void receive_file_protocol_client(int sd){ //char* buffer){
  int ret, len;
  uint16_t lmsg;
  char buffer[BUF_LEN];
  char *filename;

  //RICEVO LA LUNGHEZZA DEL FILENAME
  ret = recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  filename = (char*) malloc(len*sizeof(char));

  //RICEVO IL FILENAME
  ret = recv_all(sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", filename);

  //RICEVO IL FILE
  recv_file_b(sd, filename);

  //free(filename);
}

// *********************
// Può diventare un processo a
// se stante per l'invio di
// un file, senza rallenatre la chat
// *********************

void send_file_protocol_client(struct sockaddr_in* dest_addr, char* filename){ //char* buffer){
  int ret, len, sd;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  //APRO LA SOCKET DI INVIO
  sd = socket(AF_INET, SOCK_STREAM, 0);
  ret = connect(sd, (struct sockaddr*)dest_addr, sizeof(*dest_addr));
  if(ret<0){
    return;
  }

  //INVIO LA RICHIESTA DI SHARE
  sprintf(buffer,"%s", "SHR");
  ret = send_all(sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL FILENAME
  len = strlen(filename)+1;
  lmsg = htons(len);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL FILENAME
  sprintf(buffer,"%s", filename);
  ret = send_all(sd, (void*) buffer, len, 0);

  //INVIO IL FILE
  send_file_b(filename, sd);

  close(sd);
}

// **************
// chiedo al server
// l'elenco degli utenti
// ATTIVI
// **************

void group_protocol_client(int srv_sd){
  int ret, len;
  uint16_t lmsg;
  char buffer[BUF_LEN];
  char *username;

  //INVIO LA RICHIESTA DI GROUP
  sprintf(buffer,"%s", "GRP");
  ret = send_all(srv_sd, (void*)buffer, REQ_LEN, 0);

  while(1){
    //RICEVO LA LUNGHEZZA DELLO USERNAME
    recv_all(srv_sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg);
    if(len == 0) break;
    username = (char*) malloc(len*sizeof(char));

    //RICEVO LO USERNAME
    recv_all(srv_sd, (void*)buffer, len, 0);
    strcpy(username, buffer);

    printf("%s\n", username);
  }
}

void add_user_request_protocol_client(int cht_sd, char* username){
  int len, ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  //INVIO LA RICHIESTA DI AGGIUNTA USERNAME
  sprintf(buffer,"%s", "ADD");
  ret = send(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DELLO USERNAME
  len = strlen(username)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO LO USERNAME
  sprintf(buffer,"%s", username);
  ret = send(cht_sd, (void*) buffer, len, 0);
}

void add_user_protocol_client(int sd, int p_father_sd){
  int ret, len;
  uint16_t lmsg;
  uint32_t len32;
  char buffer[BUF_LEN];
  char *username;

  //RICEVO LA LUNGHEZZA DELLO USERNAME
  ret = recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  username = (char*) malloc(len*sizeof(char));

  //RICEVO LO USERNAME
  ret = recv_all(sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  //AGGIUNGO L'UTENTE AL CHATTING PROCESS
  sprintf(buffer, "ADD");
  write(p_father_sd, buffer, REQ_LEN);

  len32 = strlen(username)+1;
  write(p_father_sd, &len32, sizeof(uint32_t));
  sprintf(buffer, "%s", username);
  write(p_father_sd, buffer, len32);
}

void leave_chatroom_request_protocol_client(int cht_sd, char* my_username){
  int len, ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  //INVIO LA RICHIESTA DI USCITA
  sprintf(buffer,"%s", "BEY");
  ret = send(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DELLO USERNAME
  len = strlen(my_username)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO LO USERNAME
  sprintf(buffer,"%s", my_username);
  ret = send(cht_sd, (void*) buffer, len, 0);
}

void leave_chatroom_protocol_client(int sd, int p_father_sd){
  int ret, len;
  uint16_t lmsg;
  uint32_t len32;
  char buffer[BUF_LEN];
  char *username;

  //RICEVO LA LUNGHEZZA DELLO USERNAME
  ret = recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  username = (char*) malloc(len*sizeof(char));

  //RICEVO LO USERNAME
  ret = recv_all(sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  //RIMUOVO L'UTENTE DAL CHATTING PROCESS
  sprintf(buffer, "BEY");
  write(p_father_sd, buffer, REQ_LEN);
  len32 = strlen(username)+1;
  write(p_father_sd, &len32, sizeof(uint32_t));
  sprintf(buffer, "%s", username);
  write(p_father_sd, buffer, len32);
}

void join_chatroom_request_protocol_client(int cht_sd, char* my_username, struct user** chatroom_ref){
  int len, ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];
  char* username;

  //INVIO LA RICHIESTA DI UNIONE
  sprintf(buffer,"%s", "JNG");
  ret = send(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL MIO USERNAME
  len = strlen(my_username)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL MIO USERNAME
  sprintf(buffer,"%s", my_username);
  ret = send(cht_sd, (void*) buffer, len, 0);

  // ASPETTO LA LISTA DI UTENTI DA AGGIUNGERE ALLA CHATROOM
  //printf("Aspetto la lista di utenti.\n");
  while(1){
    //RICEVO LA LUNGHEZZA DELLO USERNAME
    recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
    len = ntohs(lmsg);
    if(len == 0) break;
    username = (char*) malloc(len*sizeof(char));

    //RICEVO LO USERNAME
    recv_all(cht_sd, (void*)buffer, len, 0);
    strcpy(username, buffer);

    //printf("%s\n",username);

    // LO AGGIUNGO ALLA CHATROOM
    if(strcmp(username, my_username)!=0)
      append_user(chatroom_ref, username);
  }
}

void join_chatroom_protocol_client(int sd, int p_father_sd, int p_son_sd){
  int ret;
  uint16_t lmsg;
  uint32_t len;
  char buffer[BUF_LEN];
  char *username;//, *c_user;

  //RICEVO LA LUNGHEZZA DELLO USERNAME
  ret = recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  username = (char*) malloc(len*sizeof(char));

  //RICEVO LO USERNAME
  ret = recv_all(sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", username);

  //printf("Valuto se accettare o meno.\n");

  //VALUTO SE LO USERNAME E' NELLA CHATROOM
  sprintf(buffer, "JNG");
  write(p_father_sd, buffer, REQ_LEN);
  sprintf(buffer, "%s", username);
  len = strlen(buffer)+1;
  write(p_father_sd, &len, sizeof(uint32_t));
  write(p_father_sd, buffer, strlen(buffer)+1);

  //INOLTRO UNA LISTA DI USERNAME (MANDO 0 PER CHIUDERE)
  while(1){
    // RICEVO LA LUNGHEZZA DAL CHATTING PROTOCOL
    // E LA INOLTRO TRAMITE SOCKET
    ret = read(p_son_sd, &len, sizeof(len));
    if(len == 0) break;

    // RICEVO LA LO USERNAME DAL CHATTING PROTOCOL
    // E LA INOLTRO TRAMITE SOCKET
    ret = read(p_son_sd, buffer, len);
    lmsg = htons(len);
    ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
    ret = send_all(sd, (void*) buffer, len, 0);

    //printf("Invio: %s\n", buffer);
  }
  //printf("Invio 0 peer terminare\n");
  lmsg = htons(0);
  ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);
}
