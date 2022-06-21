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

  int fdmax;

  struct sockaddr_in my_addr, cl_addr;
  char buffer[BUF_LEN];

  struct user_data *u_register = NULL;
  struct chat* l_chat = NULL;

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

  load_register(&u_register);
  load_l_chat(&l_chat);
  printf("********************** SERVER AVVIATO **********************\n");
  display_help_message();
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
          if(strcmp(buffer,"list")==0) display_list(u_register);
          else if(strcmp(buffer,"msg")==0) prind_all_hanging_msg(l_chat);
          else if(strcmp(buffer,"help")==0) display_help_message();
          else if(strcmp(buffer,"esc")==0){
            printf("Inizio la procedura di CHIUSURA...\n");
            printf("Chiudo tutte le socket...\n");
            close_all_connections(u_register);
            printf("Salvo i messaggi pendenti...\n");
            save_l_chat(l_chat);
            printf("Salvo il registro degli utenti...\n");
            save_register(u_register);
            printf("Imposto lo STATO OFF...\n");
            status = OFF;
            close(listener);
            exit(0);
          } // procedura di shutdown: chiudi socket e tutto
        }
        else{
          ret = recv_all(i, (void*)buffer, REQ_LEN, 0);
          if(ret==0){
            printf("Client Disconnesso\n");
            fflush(stdout);
            logout(&u_register, find_user_by_socket(&u_register,i));
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
            login_protocol(i, &u_register, &l_chat, buffer);
          }
          else if(strcmp(buffer,"HNG")==0){
            printf("RICHIESTA DI HANGING\n");
            hanging_protocol(i, &l_chat, buffer);
          }
          else if(strcmp(buffer,"SHW")==0){
            printf("RICHIESTA DI SHOW\n");
            show_protocol(i, &l_chat, buffer);
          }
          else if(strcmp(buffer,"CHT")==0){
            printf("RICHIESTA DI CHAT\n");
            new_chat_protocol(i, &u_register, &l_chat, buffer);
          }
          else if(strcmp(buffer,"OUT")==0){
            printf("RICHIESTA DI LOGOUT\n");
            logout_protocol(i, &u_register, buffer);
            close(i);
          }
          else if(strcmp(buffer,"GRP")==0){
            printf("RICHIESTA DI GROUP\n");
            group_protocol(i, &u_register, buffer);
          }
          else if(strcmp(buffer,"MAK")==0){
            printf("RICHIESTA DI MESSAGE ACK\n");
            forw_msg_ack_protocol(i, &u_register, &l_chat, buffer);
          }
          else if(strcmp(buffer,"ONL")==0){
            printf("RICHIESTA DI ONLINE CHECK\n");
            online_check_protocol(i, &u_register, buffer);
          }
        }
      }
    }
  }
  close(listener);
  return 0;
}
