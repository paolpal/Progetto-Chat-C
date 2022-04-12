#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "client/constants.h"
#include "client/structs.h"
#include "client/networking.h"
#include "client/protocols.h"
#include "client/chatting.h"
#include "client/input.h"

int main(int argc, char const *argv[]) {
  int nump, ret, srv_sd, listener, newfd, i, seq_n;
  int p_father_sd[2], p_son_sd[2];
  unsigned int addrlen;

  struct sockaddr_in srv_addr, my_addr, peer_addr;
  struct chat *l_chat = NULL;
  struct msg* msg;

  char buffer[BUF_LEN];
  char *dest;
  char *sender;
  char *command;
  char *number;
  char *username;
  char *password;
  char *token;
  char *msg_text;

  pid_t pid;

  short lst_port;
  short srv_port;

  fd_set master;
  fd_set read_fds;

  int fdmax;
  int chatting = 0;
  int logged = 0;

  uint32_t len;

  if(argc<2){
    printf("Per avviare l'applicazione dichiara la porta su cui ricevere i messaggi.\nUtilizzo: ./dev <port>\n");
    return 0;
  }

  lst_port = atoi(argv[1]);

  listener = socket(AF_INET, SOCK_STREAM, 0);
  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(lst_port);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));
  if(ret<0){
    perror("Bind non riuscita");
    exit(0);
  }

  listen(listener, 10);

  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  fdmax = listener;

  FD_SET(fileno(stdin), &master);
  FD_SET(listener, &master);

  //sd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  //srv_addr.sin_port = htons(4242);
  inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);

  /*ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
  if(ret<0){
    perror("Errore in fase di connessione\n");
    exit(-1);
  }*/

  display_help_message();
  while (1) {
    //if(!chatting) printf("\r<menu> ");
    //fflush(stdout);
    read_fds = master;
    select(fdmax+1, &read_fds, NULL, NULL, NULL);
    for(i=0; i<=fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
        //printf("%d\n", i);
        if(i == listener){
          addrlen = sizeof(peer_addr);
          newfd = accept(listener, (struct sockaddr*)&peer_addr, &addrlen);
          FD_SET(newfd, &master);
          if(newfd>fdmax){
            fdmax = newfd;
          }
        }
        else if(chatting && i==p_son_sd[0]){
          //printf("SONO QUA?\n");
          read(p_son_sd[0], buffer, REQ_LEN);
          if(strcmp(buffer,"MSG")==0){
            //printf("Sto archiviando i miei messaggi\n");
            // ***********************
            // RICEVO I DATI CORRETTAMENTE
            // ***********************

            // RICEVO LA LUNGHEZZA DELLO USERNAME
            read(p_son_sd[0], &len, sizeof(uint32_t));
            dest = (char*)malloc(len*sizeof(char));
            // RICEVO LO USERNAME
            read(p_son_sd[0], buffer, len);
            strcpy(dest,buffer);
            // RICEVO LA LUNGHEZZA DEL MESSAGGIO
            read(p_son_sd[0], &len, sizeof(uint32_t));
            // RICEVO IL MESSAGGIO
            read(p_son_sd[0], buffer, len);
            msg_text = buffer;
            // RICEVO IL NUMERO DI SEQUENZA
            read(p_son_sd[0], &len, sizeof(uint32_t));
            seq_n = len;

            //read(p_son_sd[0], buffer, BUF_LEN);
            //dest = strtok(buffer,":");
            //msg_text = &buffer[strlen(dest)+1];
            msg = create_my_msg(dest, msg_text, seq_n);
            //stampa_messaggio(msg);
            accoda_messaggio(&l_chat, msg);
          }
          else{
            chatting = 0;
            FD_SET(fileno(stdin), &master);
            FD_CLR(i, &master);
            close(p_son_sd[0]);
            close(p_father_sd[1]);
          }
        }
        else if(i == fileno(stdin)){
          //scanf("%s",buffer);
          fgets(buffer,BUF_LEN,stdin);
          //printf("%s\n", buffer);
          strtok(buffer, "\n"); //remove newline
          //printf("%s\n", buffer);
          nump = parametrs_num(buffer);
          command = strtok(buffer," ");
          //printf("%s %d %lu\n", command, nump, strlen(command));
          if(!logged && strcmp(command,"signup")==0 && (nump == 3)){
            //printf("RICHIESTA DI SIGNUP\n");
            //scanf("%s %s", username, password);
            //fgets(buffer,BUF_LEN,stdin);
            number = strtok(NULL, " ");
            srv_port = atoi(number);
            username = strtok(NULL, " ");
            password = strtok(NULL, " ");
            srv_addr.sin_port = htons(srv_port);
            srv_sd = socket(AF_INET, SOCK_STREAM, 0);
            ret = connect(srv_sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
            if(signup_protocol_client(srv_sd, username, password)){
              printf("Iscrizione avvenuta con successo!\n");
            }
            //free(username);
            //free(password);
            close(srv_sd);
          }
          else if(!logged && strcmp(command,"in")==0 && (nump == 3)){
            //printf("RICHIESTA DI LOGIN\n");
            //fgets(buffer,BUF_LEN,stdin);
            logged = 1;
            number = strtok(NULL, " ");
            srv_port = atoi(number);
            token = strtok(NULL, " ");
            password = strtok(NULL, " ");
            username = malloc((strlen(token)+1)*sizeof(char));
            sprintf(username,"%s",token);
            //sprintf(token,"%s",username);
            printf("<LOG> Apro una connessione TCP con il SERVER\n");
            srv_addr.sin_port = htons(srv_port);
            srv_sd = socket(AF_INET, SOCK_STREAM, 0);
            ret = connect(srv_sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
            //scanf("%hd %s %s", &port, username, password);
            if(login_protocol_client(srv_sd, username, password, lst_port)){
              printf("Login avvenuto con successo!\n");
            }
          }
          else if(logged && strcmp(command,"out")==0 && (nump == 0)){
            printf("RICHIESTA DI LOGOUT\n");
            logged = 0;
            if(logout_protocol_client(srv_sd, username)){
              printf("Logout avvenuto con successo!\n");
              close(srv_sd);
            }
          }
          else if(strcmp(command,"help")==0) display_help_message();
          // il processo principale smette di scoltare sullo STDIN
          // finche il processo di trasmissione non termina
          // la notifica avviene tramite la chiusura della pipe
          else if(logged && strcmp(command,"chat")==0 && (nump == 1)){
            //printf("RICHIESTA DI CHAT\n");
            dest = strtok(NULL, " ");
            pipe(p_son_sd);
            pipe(p_father_sd);
            chatting = 1;
            print_chat(l_chat, dest);
            FD_CLR(fileno(stdin), &master);
            FD_SET(p_son_sd[0], &master);
            if(p_son_sd[0]>fdmax){
              fdmax = p_son_sd[0];
            }
            pid = fork();
            if(pid == 0){
              close(p_son_sd[0]);
              close(p_father_sd[1]);
              chat(srv_sd, p_son_sd[1], p_father_sd[0], username, dest);
              close(p_son_sd[1]);
              close(p_father_sd[0]);
              exit(1);
            }
            close(p_son_sd[1]);
            close(p_father_sd[0]);
          }
          else if(logged && strcmp(command,"hanging")==0 && (nump == 0)){
            //printf("RICHIESTA DI HANGING\n");
            hanging_protocol_client(srv_sd, username);
          }
          else if(logged && strcmp(command,"show")==0 && (nump == 1)){
            //printf("RICHIESTA DI SHOW\n");
            sender = strtok(NULL, " ");
            show_protocol_client(srv_sd, username, sender, &l_chat);
          }
          else if(!logged && strcmp(command,"esc")==0){
            printf("Arrivederci\n");
            close(listener);
            exit(0);
            break;
          }
        }
        else {
          ret = recv_all(i, (void*)buffer, REQ_LEN, 0);
          if(ret==0){
            fflush(stdout);
            close(i);
            FD_CLR(i, &master);
            break;
          }
          if(strcmp(buffer, "MSG")==0){
            printf("<LOG-M> Ricevo richiesta di MESSAGE\n");
            recv_msg(srv_sd, i, p_father_sd[1], p_son_sd[0], chatting, username, &l_chat, buffer);
          }
          else if(strcmp(buffer, "MAK")==0){
            printf("<LOG-M> Ricevo richiesta di MESSAGE ACK\n");
            recv_msg_ack_protocol_client(i, &l_chat);
          }
          else if(strcmp(buffer, "ADD")==0){
            printf("<LOG-M> Ricevo richiesta di ADD USER\n");
            add_user_protocol_client(i, p_father_sd[1]);
          }
          else if(strcmp(buffer, "SHR")==0){
            printf("<LOG-M> Ricevo richiesta di SHARE FILE\n");
            receive_file_protocol_client(i);
          }
          else if(strcmp(buffer, "BEY")==0){
            printf("<LOG-M> Ricevo richiesta di LEAVE\n");
            leave_chatroom_protocol_client(i, p_father_sd[1]);
          }
          else if(strcmp(buffer, "JNG")==0){
            printf("<LOG-M> Ricevo richiesta di JOIN (SINCRONIZZAZIONE)\n");
            join_chatroom_protocol_client(i, p_father_sd[1],  p_son_sd[0]);
          }
        }
      }
    }
  }

  close(srv_sd);
  return 0;
}
