#ifndef _STRUCTS_H_
#define _STRUCTS_H_

struct user_data{
  char user_dest[50];
  short port;
  int sd;
  time_t* timestamp_login;
  time_t* timestamp_logout;

  struct user_data *next;
};

struct hanging_msg{
  char text[BUF_LEN];
  char send[50];
  char dest[50];
  int seq_n;
  time_t *timestamp;
  struct hanging_msg *next;
};

struct destinatario{
  char name[50];
  struct hanging_msg *messaggi;
  struct destinatario *next;
};

struct sender{
  char username[50];
  int n_msg;
  struct sender* next;
};

#endif
