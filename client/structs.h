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
  char user[NAME_LEN];
  short port;
  struct sockaddr_in addr;
};

struct msg{
  char text[BUF_LEN];
  char sender[NAME_LEN];
  int ACK;
  int seq_n;
  char dest[NAME_LEN];
  struct msg *next;
};

struct chat{ // chat - mittente
  char name[NAME_LEN]; // user - mittente
  struct msg *l_msg;
  struct chat *next;
};

struct user{
  char name[NAME_LEN];
  int cht_sd;
  int next_seq_n;
  struct sockaddr_in addr;
  struct user* next;
};

#endif
