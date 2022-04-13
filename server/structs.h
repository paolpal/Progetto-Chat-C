#ifndef _STRUCTS_H_
#define _STRUCTS_H_

struct user_data{
  char *user_dest;
  short port;
  int sd;
  time_t* timestamp_login;
  time_t* timestamp_logout;

  struct user_data *next;
};

struct hanging_msg{
  char *msg;
  char *send;
  char *dest;
  int seq_n;
  time_t *timestamp;
  struct hanging_msg *next;
};

struct destinatario{
  char *destinatario;
  struct hanging_msg *messaggi;
  struct destinatario *next;
};

struct sender{
  char* username;
  int n_msg;
  struct sender* next;
};

#endif
