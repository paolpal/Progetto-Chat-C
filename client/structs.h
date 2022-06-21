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

#include "constants.h"

struct msg{
  char text[BUF_LEN];
  char sender[S_BUF_LEN];
  char dest[S_BUF_LEN];
  int ACK;
  int seq_n;
  struct msg *next;
};

struct chat{
  char name[S_BUF_LEN];
  struct msg *l_msg;
  int next_seq_n;
  struct chat *next;
};

struct user{
  char name[S_BUF_LEN];
  int cht_sd;
  struct chat* chat; //riferimento alla lista, per praticità
  struct sockaddr_in addr;
  struct user* next;
};

#endif
