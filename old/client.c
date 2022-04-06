#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUF_LEN 1024
#define REQ_LEN 4
#define ACK_LEN 7

int signup_protocol_client(int sd, char* user, char* pw);

int main(int argc, char const *argv[]) {
  int ret, sd;
  //uint16_t lmsg;

  //short int port = atoi(argv[1]); //>65535?0:atoi(argv[1]);
  //printf("%lu\n", sizeof(htons(port)));

  struct sockaddr_in srv_addr;
  //char buffer[BUF_LEN];

  char* cmds[3] = {"SGN\0", "LIN\0", "OUT\0"};

  sd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(4242);
  inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

  ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));


  if(ret<0){
    perror("Errore in fase di connessione");
    exit(-1);
  }
  //sleep(1);

  //lmsg = htons(1545);

  //ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);

  char user[20];// = "paolo";
  char pw[20];// = "123123";

  do{
    printf("Inserisci nome e password:\n");
    scanf("%s %s", user, pw);
  } while(!signup_protocol_client(sd,user,pw));

  close(sd);
  return 0;
}

int signup_protocol_client(int sd, char* user, char* pw){
  int ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  // FACCIO RICHIESTA DI ISCRIZIONE
  sprintf(buffer,"%s", "SGN\0");
  printf("Invio richiesta di ISCRZIONE\n");
  ret = send(sd, (void*)buffer, REQ_LEN, 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)REQ_LEN-ret,ret);
  if(ret<0){
    perror("Errore in fase di invio del comando");
    exit(-1);
  }
  //RICHIESTA ACCETTTATA
  printf("Attendo l'ACK\n");
  ret = recv(sd, (void*)buffer, ACK_LEN,0);
  printf("%s\n\n", buffer);

  //INVIO LA LUNGHEZZA DEL NOME
  int len = strlen(user)+1;
  lmsg = htons(len);
  printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);
  if(ret<0){
    perror("Errore in fase di invio del comando");
    exit(-1);
  }
  //LUNGHEZZA RICEVUTA
  printf("Attendo l'ACK\n");
  ret = recv(sd, (void*)buffer, ACK_LEN, 0);
  printf("%s\n\n", buffer);

  //INVIO IL NOME
  printf("Invio lo username");
  sprintf(buffer,"%s", user);
  ret = send(sd, (void*) buffer, len, 0);
  printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);
  if(ret<0){
    perror("Errore in fase di invio del comando");
    exit(-1);
  }
  //NOME RICEVUTO
  printf("Attendo l'ACK\n");
  ret = recv(sd, (void*)buffer, ACK_LEN, 0);
  printf("%s\n\n", buffer);

  //INVIO LA LUNGHEZZA DELLA password
  len = strlen(pw)+1;
  lmsg = htons(len);
  printf("LUNGHEZZA password :%d (%d)\n", len, (int)lmsg);
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);
  if(ret<0){
    perror("Errore in fase di invio del comando");
    exit(-1);
  }
  //LUNGHEZZA RICEVUTA
  printf("Attendo l'ACK\n");
  ret = recv(sd, (void*)buffer, ACK_LEN, 0);
  printf("%s\n\n", buffer);

  //INVIO LA PASSWORD
  printf("Invio la password");
  sprintf(buffer,"%s", pw);
  ret = send(sd, (void*) buffer, len, 0);
  printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);
  if(ret<0){
    perror("Errore in fase di invio del comando");
    exit(-1);
  }
  //PROCEDURA TERMINATA
  printf("Attendo l'ACK\n");
  ret = recv(sd, (void*)buffer, ACK_LEN, 0);
  printf("%s\n", buffer);

  if(strcmp(buffer,"PSWACK")==0)return 1;
  else return 0;
}
