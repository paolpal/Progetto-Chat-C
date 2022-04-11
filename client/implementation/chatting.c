#include "../chatting.h"
#include "../networking.h"
#include "../protocols.h"
#include "../utility.h"

// il processo figlio scrive nella pipe SON
// e ascolta su FATHER

void chat(int srv_sd, int p_son_sd, int p_father_sd,char* my_user, char* dest_user){
  char buffer[BUF_LEN];
  char msg[BUF_LEN];
  char *filename;
  char *username;
  char cmd[6];
  char sh_cmd[3];
  int i;
  uint32_t len;
  struct user* chatroom = NULL;
  struct user* user = NULL;
  //int cht_sd = 0;
  int chatting = 1;

  int fdmax;

  fd_set master;
  fd_set read_fds;

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  fdmax = p_father_sd;

  FD_SET(fileno(stdin), &master);
  FD_SET(p_father_sd, &master);

  //struct contatto dest;
  //struct sockaddr_in dest_addr;
  //dest.user = dest_user;
  //dest.port = 0;

  append_user(&chatroom, dest_user);

  //if(chatroom==NULL) printf("TUTTO A PUTTANE\n");
  //printf("Hai iniziato una chat con %s\n", dest_user);

  while(chatting){
    //printf("\r> ");
    //fflush(stdout);
    read_fds = master;
    select(fdmax+1, &read_fds, NULL, NULL, NULL);
    for(i=0; i<=fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
        if(i==p_father_sd){
          //printf("RICHIESTA DA MAIN PROCESS\n");
          // CHK : ricevo un nome e rispondo 1 se sto chattando, 0 se non sto chattando con lui
          read(p_father_sd, buffer, REQ_LEN);
          if(strcmp(buffer,"CHK")==0){
            read(p_father_sd, &len, sizeof(uint32_t));
            read(p_father_sd, buffer, len);
            //read(p_father_sd, buffer, BUF_LEN);
            if(chatting_with(buffer, chatroom)) {
              write(p_son_sd,"1\0", 2);
            }
            else {
              write(p_son_sd,"0\0", 2);
            }
          }
          else if(strcmp(buffer,"ADD")==0){
            read(p_father_sd, &len, sizeof(uint32_t));
            read(p_father_sd, buffer, len);
            //printf("%s %d\n", buffer, (int)strlen(buffer));
            if(strcmp(my_user, buffer)!=0)
              append_user(&chatroom, buffer);
          }
          else if(strcmp(buffer,"BEY")==0){
            read(p_father_sd, &len, sizeof(uint32_t));
            read(p_father_sd, buffer, len);
            //printf("%s %d\n", buffer, (int)strlen(buffer));
            remove_user(&chatroom, buffer);
          }
          else if(strcmp(buffer,"JNG")==0){
            read(p_father_sd, &len, sizeof(uint32_t));
            read(p_father_sd, buffer, len);
            //read(p_father_sd, buffer, BUF_LEN);
            //printf("%s %d\n", buffer, (int)strlen(buffer));
            if(chatting_with(buffer, chatroom)){
              send_chatroom_mp(p_son_sd, chatroom);
            }
          }
          //printf("RISPOSTO AL MAIN PROCESS\n");
        }
        else{
          //printf("RICHIESTA DA STDIN\n");
          fgets(buffer, BUF_LEN, stdin);
          strncpy(cmd, buffer, 5);
          strncpy(sh_cmd, buffer, 2);
          cmd[5] = '\0';
          sh_cmd[2] = '\0';
          if(strcmp(sh_cmd,"\\q")==0){
            chatting=0;
            user = chatroom;
            while(user!=NULL){
              if(user->cht_sd != 0){
                leave_chatroom_request_protocol_client(user->cht_sd, my_user);
              }
              user = user->next;
            }
          }
          else if(strcmp(sh_cmd,"\\p")==0) print_chatroom(chatroom);
          else if(strcmp(sh_cmd,"\\u")==0) group_protocol_client(srv_sd);
          else if(strcmp(sh_cmd,"\\j")==0){
            user = chatroom;
            while(user!=NULL){
              if(user->cht_sd != 0){
                join_chatroom_request_protocol_client(user->cht_sd, my_user, &chatroom);
              }
              user = user->next;
            }
          }
          else if(strcmp(sh_cmd,"\\a")==0) {
            strtok(buffer, " ");
            username = strtok(NULL, "\n");
            //username = strtok(username, "\n");
            //printf("%s %d\n", username, (int)strlen(username));
            user = chatroom;
            while(user!=NULL){
              if(user->cht_sd != 0){
                add_user_request_protocol_client(user->cht_sd, username);
              }
              user = user->next;
            }
            // STAMPA CHATROOM
            append_user(&chatroom, username);
            //print_chatroom(chatroom);
          }
          else if(strcmp(cmd,"share")==0){
            strtok(buffer, " ");
            filename = strtok(NULL, " ");
            user = chatroom;
            while(user!=NULL){
              if(user->cht_sd != 0){
                send_file_protocol_client(&user->addr, filename);
              }
              user = user->next;
            }
          }
          else {
            // INVIO MESSAGGIO NORMALE
            user = chatroom;
            strcpy(msg, buffer);
            while(user!=NULL){
              if(user->cht_sd != 0){
                send_msg(user->cht_sd, my_user, msg);
              }
              else {
                user->cht_sd = new_chat_protocol_client(srv_sd, my_user, user->username, &user->addr, msg);
              }
              //printf("****** DEBUG 1 ******\n");
              sprintf(buffer, "MSG");
              write(p_son_sd, buffer, REQ_LEN);
              //printf("****** DEBUG 2 ******\n");
              // SIGNORE E SIGNORI ECCO IL BUG

              // MANDO LA LUNGHEZZA DELLO USERNAME
              len = strlen(user->username)+1;
              write(p_son_sd, &len, sizeof(uint32_t));
              // MANDO LO USERNAME
              write(p_son_sd, user->username, len);
              // MANDO LA LUNGHEZZA DEL MESSAGGIO
              len = strlen(msg)+1;
              write(p_son_sd, &len, sizeof(uint32_t));
              // MANDO IL MESSAGGIO
              write(p_son_sd, msg, len);
              // MANDO IL NUMERO DI SEQUENZA
              // --- len = NUMERO SEQUENZA;
              // --- write(p_son_sd, &len, sizeof(uint32_t));

              //sprintf(buffer, "%s:%s", user->username, msg);
              //write(p_son_sd, buffer, strlen(buffer)+1);
              //printf("****** DEBUG 3 ******\n");
              user = user->next;
            }
          }
        }
      }
    }
  }
  sprintf(buffer, "END");
  write(p_son_sd, buffer, REQ_LEN);
  //if(cht_sd != 0) close(cht_sd);
}

void send_msg(int cht_sd, char* my_user, char* msg){
  int len, ret;
  uint16_t lmsg;
  char buffer[BUF_LEN];

  //INVIO LA RICHIESTA DI MESSAGGIO
  sprintf(buffer, "MSG");
  ret = send(cht_sd, (void*)buffer, REQ_LEN, 0);

  //INVIO LA LUNGHEZZA DEL MITTENTE
  len = strlen(my_user)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL MITTENTE
  sprintf(buffer,"%s", my_user);
  ret = send(cht_sd, (void*) buffer, len, 0);

  //INVIO LA LUNGHEZZA DEL MESSAGGIO
  len = strlen(msg)+1;
  lmsg = htons(len);
  ret = send(cht_sd, (void*) &lmsg, sizeof(uint16_t), 0);

  //INVIO IL MESSAGGIO
  ret = send(cht_sd, (void*) msg, len, 0);
  return;
}

void recv_msg(int cht_sd, int p_father_sd, int p_son_sd, int chatting, struct chat** ricevuti, char* buffer){
  uint16_t lmsg;
  uint32_t len_t;
  int len;
  struct msg* messaggio = (struct msg*) malloc(sizeof(struct msg));

  messaggio->dest = NULL;

  //RICEVO LA LUNGHEZZA DEL MITTENTE
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  messaggio->sender = (char*) malloc(len*sizeof(char));
  messaggio->next = NULL;

  //RICEVO IL MITTENTE
  //printf("Ricevo username\n");
  recv_all(cht_sd, (void*)buffer, len, 0);
  sscanf(buffer, "%s", messaggio->sender);
  //printf("%s\n", username);

  //RICEVO LA LUNGHEZZA DEL MESSAGGIO
  //printf("Ricevo lunghezza password\n");
  recv_all(cht_sd, (void*)&lmsg, sizeof(uint16_t), 0);
  len = ntohs(lmsg);
  messaggio->text = (char*) malloc(len*sizeof(char));

  //RICEVO IL MESSAGGIO
  //printf("Ricevo password\n");
  recv_all(cht_sd, (void*)buffer, len, 0);
  //sscanf(buffer, "%s", messaggio->msg);
  strcpy(messaggio->text,buffer);

  //COMUNICO CON IL CHATTING PROCESS SE ATTIVO
  if(chatting){
    //printf("Chiedo se sta chattando con USER\n");
    sprintf(buffer, "CHK");
    write(p_father_sd, buffer, REQ_LEN);
    len_t = strlen(messaggio->sender)+1;
    write(p_father_sd, &len_t, sizeof(uint32_t));
    write(p_father_sd, messaggio->sender, len_t);
    //printf("Attendo la risposta\n");
    //p_read_all(p_son_sd, buffer, sizeof(buffer));
    read(p_son_sd, buffer, 2);
    //printf("Ho ricevuto la risposta : %s\n", buffer);
    if(strcmp(buffer, "1")==0)
      stampa_messaggio(messaggio);
  }
  accoda_messaggio(ricevuti, messaggio);

}

void accoda_messaggio(struct chat **l_chat, struct msg *msg){
  char* find = (msg->sender==NULL)? msg->dest:msg->sender;
  //printf("%s\n", find);
  struct chat *c_chat = *l_chat;
  while(c_chat!=NULL){
    if(strcmp(c_chat->user, find)==0){
      push_msg(&c_chat->l_msg, msg);
      return;
    }
    c_chat = c_chat->next;
  }
  //printf("NON HO TROVATO NULLA");
  add_chat(l_chat, find);
  c_chat = *l_chat;
  push_msg(&c_chat->l_msg, msg);
}

void add_chat(struct chat **l_chat, char* user){
  struct chat *new_chat  = (struct chat*) malloc(sizeof(struct chat));
  new_chat->user = (char*) malloc(strlen(user)+1); // Size of char = 1
  strcpy(new_chat->user, user);
  new_chat->l_msg=NULL;
  new_chat->next = (*l_chat);
  (*l_chat) = new_chat;
}

void push_msg(struct msg **l_msg, struct msg *msg){
  if(*l_msg==NULL){
    msg->next = (*l_msg);
    (*l_msg) = msg;
  }
  else push_msg(&(*l_msg)->next, msg);
}

void stampa_messaggio(struct msg *msg){
  //char c;
  if(msg->sender != NULL)
    printf("\r%s : %s", msg->sender, msg->text);
  else
    printf("\r* %s", msg->text);
  //while ((c=getchar()) != '\n')putchar(c);
  //while(!feof(stdin))putchar(getchar());
  fflush(stdout);
}

void print_chat(struct chat *l_chat, char* user){
  struct chat *c_chat = l_chat;
  struct msg *c_msg;
  while(c_chat!=NULL){
    if(strcmp(c_chat->user, user)==0){
      c_msg = c_chat->l_msg;
      while(c_msg!=NULL){
        stampa_messaggio(c_msg);
        c_msg = c_msg->next;
      }
    }
    c_chat = c_chat->next;
  }
}
