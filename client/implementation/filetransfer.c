#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../filetransfer.h"
#include "../networking.h"

void recv_file(int sockfd, char* filename){
  int n;
  FILE *fp;
  char buffer[BUF_LEN];

  fp = fopen(filename, "w");
  if(fp==NULL){
    perror("[-]Error in creating file.");
    exit(1);
  }
  while(1){
    n = recv(sockfd, buffer, BUF_LEN, 0);
    if(n<=0){
      break;
      return;
    }
    fprintf(fp, "%s", buffer);
    bzero(buffer, BUF_LEN);
  }
  fclose(fp);
  return;
}

void send_file(char* filename, int sockfd){
  FILE *fp;
  char data[BUF_LEN] = {0};
  strtok(filename, "\n");

  fp = fopen(filename, "r");
  if(fp==NULL){
    perror("[-]Error in opening file.");
    exit(1);
  }
  while(fgets(data, BUF_LEN, fp)!=NULL){
    if(send(sockfd, data, sizeof(data), 0)== -1){
      perror("[-] Error in sendung data");
      exit(1);
    }
    bzero(data, BUF_LEN);
  }
  fclose(fp);
}

void recv_file_b(int sd, char* filename){
  int len, ret;
  uint16_t lmsg;
  FILE *fp;
  char buffer[BUF_LEN] = {0};

  fp = fopen(filename, "wb");
  if(fp==NULL){
    perror("[-]Error in creating file.");
    exit(1);
  }
  while(1){
    //RICEVO LA LUNGHEZZA DEL FILENAME
    ret = recv_all(sd, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret<=0){
      break;
      return;
    }

    len = ntohs(lmsg);
    ret = recv_all(sd, buffer, len, 0);
    fwrite(buffer, sizeof(char), len, fp);
    bzero(buffer, BUF_LEN);
  }
  fclose(fp);
  return;
}

void send_file_b(char* filename, int sd){
  FILE *fp;
  uint16_t lmsg;
  char data[BUF_LEN] = {0};
  int r_byte, ret;

  strtok(filename, "\n");

  fp = fopen(filename, "rb");
  if(fp==NULL){
    perror("[-]Error in opening file.");
    exit(1);
  }
  while(!feof(fp)){
    r_byte = fread(data, sizeof(char), BUF_LEN, fp);
    lmsg = htons(r_byte);
    ret = send_all(sd, (void*) &lmsg, sizeof(uint16_t), 0);

    ret = send_all(sd, data, r_byte, 0);
    if(ret == -1){
      perror("[-] Error in sendung data");
      exit(1);
    }
    bzero(data, BUF_LEN);
  }
  fclose(fp);
}
