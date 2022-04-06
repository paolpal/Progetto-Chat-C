#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "server/protocols.h"
#include "server/constants.h"
#include "server/networking.h"
#include "server/utility.h"

int main(int argc, char const *argv[]) {

  int ret, newfd, listener, i;
  unsigned int addrlen;

  volatile int status = ON;

  fd_set master;
  fd_set read_fds;

  //char *user;
  //char *pw;

  //struct protocol_info *connection = NULL;

  int fdmax;

  struct sockaddr_in my_addr, cl_addr;
  char buffer[BUF_LEN];

  struct user_data *registro = NULL;
  struct destinatario* destinatari = NULL;

  listener = socket(AF_INET, SOCK_STREAM, 0);
  memset(&my_addr, 0, sizeof(my_addr));

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = (argc<2)? htons(4242) : htons((short)atoi(argv[1]));
  my_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));
  if(ret<0){
    perror("Bind non riuscita");
    exit(0);
  }

  listen(listener, 10);

  FD_ZERO(&master);
  FD_ZERO(&read_fds);


  FD_SET(fileno(stdin), &master);
  FD_SET(listener, &master);

  fdmax = listener;

  printf("********************** SERVER AVVIATO **********************\n");

  while(status == ON){
    read_fds = master;
    select(fdmax+1, &read_fds, NULL, NULL, NULL);
    for(i=0; i<=fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
        if(i == listener){
          addrlen = sizeof(cl_addr);
          newfd = accept(listener, (struct sockaddr*)&cl_addr, &addrlen);
          FD_SET(newfd, &master);
          if(newfd>fdmax){
            fdmax = newfd;
          }
        }
        else if(i == fileno(stdin)){
          scanf("%s", buffer);
          if(strcmp(buffer,"list")==0) display_list(registro);
          else if(strcmp(buffer,"msg")==0)prind_all_hanging_msg(destinatari);
          else if(strcmp(buffer,"help")==0){} //display_help_mesage();
          else if(strcmp(buffer,"esc")==0){
            status = OFF;
          } // procedura di shutdown: chiudi socket e tutto
        }
        else{
          ret = recv_all(i, (void*)buffer, REQ_LEN, 0);
          //printf("%s\n", buffer);
          if(ret==0){
            printf("Client Disconnesso\n");
            fflush(stdout);
            close(i);
            FD_CLR(i, &master);
            break;
          }
          if(strcmp(buffer,"SGN")==0){
            printf("RICHIESTA DI SIGNUP\n");
            signup_protocol(i, buffer);
          }
          else if(strcmp(buffer,"LIN")==0){
            printf("RICHIESTA DI LOGIN\n");
            login_protocol(i, &registro, buffer);
          }
          else if(strcmp(buffer,"HNG")==0){
            printf("RICHIESTA DI HANGING\n");
            hanging_protocol(i, &destinatari, buffer);
          }
          else if(strcmp(buffer,"SHW")==0){
            printf("RICHIESTA DI SHOW\n");
            show_protocol(i, &destinatari, buffer);
          }
          else if(strcmp(buffer,"CHT")==0){
            printf("RICHIESTA DI CHAT\n");
            new_chat_protocol(i, &registro, &destinatari, buffer);
          }
          else if(strcmp(buffer,"OUT")==0){
            printf("RICHIESTA DI LOGOUT\n");
            logout_protocol(i, &registro, buffer);
          }
          else if(strcmp(buffer,"GRP")==0){
            printf("RICHIESTA DI GROUP\n");
            group_protocol(i, &registro, buffer);
          }
        }
      }
    }
  }
  close(listener);
  return 0;
}
