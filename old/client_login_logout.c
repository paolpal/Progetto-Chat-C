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

int login_protocol_client(int sd, char* user, char* pw, short port);
int logout_protocol_client(int sd, char* user);

int main(int argc, char const *argv[]) {
  int ret, sd, i;

  struct sockaddr_in srv_addr;

  char* cmds[3] = {"SGN\0", "LIN\0", "OUT\0"};

  sd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(4242);
  inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

  ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));


  if(ret<0){
    perror("Errore in fase di connessione\n");
    exit(-1);
  }

  char user[20];
  char pw[20];

  char *names[] = {"pippo", "pluto", "paperino", "paperina", "topolino", "topolina"};
  char *pws[] = {"pippo", "pluto", "paperino", "paperina", "topolino", "topolina"};

  /*do{
    printf("Inserisci nome e password:\n");
    scanf("%s %s", user, pw);
  } while(!signup_protocol_client(sd,user,pw));
  */

  for(i=0;i<6;i++){
    login_protocol_client(sd, names[i], pws[i], i+12);
  }

  for(i=0;i<6;i++){
    logout_protocol_client(sd, names[i]);
  }

  close(sd);
  return 0;
}

int login_protocol_client(int sd, char* user, char* pw, short port){
  int ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  // FACCIO RICHIESTA DI LOGIN
  sprintf(buffer,"%s", "LIN");
  printf("Invio richiesta di LOGIN\n");
  ret = send(sd, (void*)buffer, REQ_LEN, 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)REQ_LEN-ret,ret);


  //INVIO LA LUNGHEZZA DEL NOME
  int len = strlen(user)+1;
  lmsg = htons(len);
  printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO IL NOME
  printf("Invio lo username\n");
  sprintf(buffer,"%s", user);
  ret = send(sd, (void*) buffer, len, 0);
  printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);


  //INVIO LA LUNGHEZZA DELLA PASSWORD
  len = strlen(pw)+1;
  lmsg = htons(len);
  printf("LUNGHEZZA password :%d (%d)\n", len, (int)lmsg);
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO LA PASSWORD
  printf("Invio la password\n");
  sprintf(buffer,"%s", pw);
  ret = send(sd, (void*) buffer, len, 0);
  printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  //INVIO PORTA
  lmsg = htons(port);
  printf("Invio la porta\n");
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);

  //printf("%s\n", buffer);

  ret = recv(sd, (void*) buffer, ACK_LEN, 0);

  if(strcmp(buffer,"LOGGED")==0)return 1;
  else return 0;
}

int logout_protocol_client(int sd, char* user){
  int ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  // FACCIO RICHIESTA DI LOGOUT
  sprintf(buffer,"%s", "OUT");
  printf("Invio richiesta di LOGOUT\n");
  ret = send(sd, (void*)buffer, REQ_LEN, 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)REQ_LEN-ret,ret);


  //INVIO LA LUNGHEZZA DEL NOME
  int len = strlen(user)+1;
  lmsg = htons(len);
  printf("LUNGHEZZA username : %d (%d)\n", len, (int)lmsg);
  ret = send(sd, (void*) &lmsg, sizeof(uint16_t), 0);
  printf("Mancano da inviare %d (inviati %d)\n",(int)sizeof(uint16_t)-ret,ret);


  //INVIO IL NOME
  printf("Invio lo username\n");
  sprintf(buffer,"%s", user);
  ret = send(sd, (void*) buffer, len, 0);
  printf("Mancano da inviare %d (inviati %d)\n",len-ret,ret);

  ret = recv(sd, (void*) buffer, ACK_LEN, 0);

  if(strcmp(buffer,"LOGGED")==0)return 1;
  else return 0;
}
