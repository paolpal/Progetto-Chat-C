#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/*#include "structs.h"
#include "constants.h"
#include "networking.h"
#include "access.h"*/

void signup_protocol(int i, struct protocol_info *connection, char *buffer){
  uint16_t lmsg;
  int ret;
  switch(connection[i].step){
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
      strcpy(connection[i].activity, "IDL");
      break;
  }
}
