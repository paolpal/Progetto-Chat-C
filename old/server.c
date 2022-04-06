#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


#include "constants.h"
#include "networking.h"
#include "access.h"
#include "protocols.h"

int main(int argc, char const *argv[]) {

  int ret, newfd, listener, i;
  unsigned int len, addrlen;

  fd_set master;
  fd_set read_fds;

  char *user;
  char *pw;

  struct protocol_info *connection = NULL;

  int fdmax;

  struct sockaddr_in my_addr, cl_addr;
  char buffer[BUF_LEN];

  struct user_data *registro = NULL;

  listener = socket(AF_INET, SOCK_STREAM, 0);
  memset(&my_addr, 0, sizeof(my_addr));

  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(4242);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));
  if(ret<0){
    perror("Bind non riuscita");
    exit(0);
  }

  listen(listener, 10);

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  FD_SET(listener, &master);

  fdmax = listener;

  while(1){
    read_fds = master;
    select(fdmax+1, &read_fds, NULL, NULL, NULL);
    for(i=0; i<=fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
        sleep(1);
        if(i == listener){
          addrlen = sizeof(cl_addr);
          newfd = accept(listener, (struct sockaddr*)&cl_addr, &addrlen);
          FD_SET(newfd, &master);
          if(newfd>fdmax){
            fdmax = newfd;
            connection = realloc(connection, sizeof(struct protocol_info)*fdmax);
          }
          strcpy(connection[newfd].activity, "IDL");
          connection[newfd].step = 0;
          connection[newfd].to_recive = REQ_LEN;
        }
        else{
          printf("%d %s\n", i, connection[i].activity);
          if(strcmp(connection[i].activity, "IDL")==0){
            ret = recv_all(i, (void*)buffer, connection[i].to_recive, 0);
            if(ret==0){
              printf("Client Disconnesso\n");
              fflush(stdout);
              close(i);
              FD_CLR(i, &master);
              break;
            }
            printf("mancano da ricevere %d (ricevuti %d)\n",connection[i].to_recive-ret, ret);
            if(strcmp(buffer,"SGN")==0){
              strcpy(connection[i].activity, "SGN");
              signup_protocol(i,connection,buffer);
              /*strcpy(connection[i].activity, "SGN");
              connection[i].signup_info = (struct signup_data*)malloc(sizeof(struct signup_data));
              printf("%s\n", connection[i].activity);
              connection[i].step = 1;
              sprintf(buffer,"%s", "SGNACK");
              send_all(i,(void*) buffer, ACK_LEN, 0);*/
            }
            else if(strcmp(buffer,"LIN")==0){
              printf("> Login\n");
            }
            else if(strcmp(buffer,"HNG")==0){}
            else if(strcmp(buffer,"SHW")==0){}
            else if(strcmp(buffer,"CHT")==0){}
            else if(strcmp(buffer,"SHR")==0){}
            else if(strcmp(buffer,"OUT")==0){
              printf("> Logout\n");
            }
          }
          else if(strcmp(connection[i].activity, "SGN")==0){
            signup_protocol(i, connection, buffer);
            /*switch(connection[i].step){
              case 0:
                strcpy(connection[i].activity, "SGN");
                connection[i].signup_info = (struct signup_data*)malloc(sizeof(struct signup_data));
                connection[i].step = 1;
                sprintf(buffer,"%s", "SGNACK");
                send_all(i,(void*) buffer, ACK_LEN, 0);
                break;
              case 1:
                printf("Ricevo la lunghezza del nome : ");
                ret = recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
                //printf("mancano da ricevere %d (ricevuti %d)\n", (int)sizeof(uint16_t)-ret, ret);
                connection[i].to_recive = ntohs(lmsg);
                sprintf(buffer,"%s", "LENACK");
                send_all(i,(void*) buffer, ACK_LEN, 0);
                printf("%d (%d)\n\n", connection[i].to_recive, lmsg);
                connection[i].step++;
                break;
              case 2:
                printf("Ricevo il nome : ");
                (connection[i].signup_info)->username = (char*) malloc(sizeof(char)*(connection[i].to_recive));
                ret = recv_all(i, (void*)buffer, connection[i].to_recive, 0);
                //printf("mancano da ricevere %d (ricevuti %d)\n", connection[i].to_recive-ret, ret);
                sscanf(buffer, "%s", (connection[i].signup_info)->username);
                printf("%s\n\n", (connection[i].signup_info)->username);
                sprintf(buffer, "%s", "USRACK");
                send_all(i, (void*) buffer, ACK_LEN,0);
                connection[i].step++;
                break;
              case 3:
                printf("Ricevo la lunghezza della password : ");
                ret = recv_all(i, (void*)&lmsg, sizeof(uint16_t), 0);
                //printf("mancano da ricevere %d (ricevuti %d)\n",(int)sizeof(uint16_t)-ret, ret);
                connection[i].to_recive = ntohs(lmsg);
                sprintf(buffer, "%s", "LENACK");
                send_all(i,(void*) buffer, ACK_LEN,0);
                printf("%d (%d)\n\n", connection[i].to_recive, lmsg);
                connection[i].step++;
                break;
              case 4:
                printf("Ricevo la password : ");
                (connection[i].signup_info)->password = (char*) malloc(sizeof(char)*(connection[i].to_recive));
                ret = recv_all(i, (void*)buffer, connection[i].to_recive, 0);
                //printf("mancano da ricevere %d (ricevuti %d)\n",connection[i].to_recive-ret, ret);
                sscanf(buffer, "%s", (connection[i].signup_info)->password);
                printf("%s\n\n", (connection[i].signup_info)->password);
                if(signup(connection[i].signup_info->username, connection[i].signup_info->password)){
                  sprintf(buffer, "%s", "PSWACK");
                }
                else{
                  sprintf(buffer, "%s", "PSNACK");
                }
                send_all(i, (void*) buffer, ACK_LEN,0);
                free(connection[i].signup_info->username);
                free(connection[i].signup_info->password);
                free(connection[i].signup_info);
                strcpy(connection[newfd].activity, "IDL");
                break;
            }*/
          }
        }
      }
    }
  }
  close(listener);
  return 0;
}
