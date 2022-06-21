#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include "constants.h"

struct user_data{
  char username[S_BUF_LEN];
  short port;
  int sd;
  time_t t_login;
  time_t t_logout;

  struct user_data *next;
};

struct hanging_msg{
  char text[BUF_LEN];
  char send[S_BUF_LEN];
  char dest[S_BUF_LEN];
  int seq_n;
  time_t timestamp;
  struct hanging_msg *next;
};

struct msg_ack{
  char dest[S_BUF_LEN];
  int seq_n;
  struct msg_ack* next;
};

// messaggi che devo ricevere
// ack che devo ricevere
struct chat{
  char name[S_BUF_LEN];
  struct hanging_msg *l_msg;
  struct msg_ack *l_ack;
  struct chat *next;
};

struct sender{
  char username[S_BUF_LEN];
  int n_msg;
  struct sender* next;
};

#endif
