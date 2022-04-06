#ifndef _STRUCTS_H_
#define _STRUCTS_H_

struct user_data{
  char *user_dest;
  short port;
  time_t* timestamp_login;
  time_t* timestamp_logout;

  struct user_data *next;
};

struct signup_data{
  char* username;
  char* password;
};

struct protocol_info{
  char activity[4];
  int step;
  int to_recive;
  struct signup_data* signup_info;
};

struct hanging_msg{
  char *msg;
  char *send;
  char *dest;
  //time_t *timestamp;
  struct hanging_msg *next;
};

struct destinatario{
  char *destinatario;
  //int quanti;
  time_t *timestamp;
  struct hanging_msg *messaggi;
  struct destinatario *next;
};

struct sender{
  char* username;
  int n_msg;
  struct sender* next;
};

#endif
