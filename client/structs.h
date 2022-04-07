#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifndef _CLIENT_STRUCT_H_
#define _CLIENT_STRUCT_H_

struct contatto{
  char *user;
  short port;
  struct sockaddr_in addr;
};

struct msg{
  char *text;
  char *sender;
  int ACK;
  int seq_n;
  char *dest;
  //time_t *timestamp;
  struct msg *next;
};

struct chat{ // chat - mittente
  char *user; // user - mittente
  //int quanti;
  //time_t *timestamp;
  struct msg *l_msg;
  struct chat *next;
};

struct user{
  char* username;
  int cht_sd;
  struct sockaddr_in addr;
  struct user* next;
};

#endif
