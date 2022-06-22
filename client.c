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

int main(int argc, char const *argv[]) {
  int ret, srv_sd, listener=0, fdmax, nump, newfd, i;
  unsigned int addrlen;

  struct sockaddr_in srv_addr, my_addr, peer_addr;
  struct chat *l_chat = NULL;
  struct msg* msg;

  struct user* chatroom = NULL;
  struct user* user = NULL;

  char buffer[BUF_LEN];
  char msg_b[BUF_LEN];
  char logged_username[S_BUF_LEN];
  char *dest;
  char *sender;
  char *username;
  char *password;
  char *command;
  char *number;
  char *filename;
  //char *token;
  char cmd[6];
  char sh_cmd[3];

  short srv_port, lst_port;

  fd_set master, read_fds;

  int chatting = 0;
  int logged = 0;

  if(argc<2){
    printf("Per avviare l'applicazione specifica la porta di ascolto.\nUtilizzo: ./dev <port>\n");
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

  FD_SET(fileno(stdin), &master);
  FD_SET(listener, &master);

  fdmax = listener;

  display_help_message();

  while(1){
    read_fds = master;
    select(fdmax+1, &read_fds, NULL, NULL, NULL);
    for(i=0; i<=fdmax; i++){
      if(FD_ISSET(i, &read_fds)){
        if(logged && i == listener){
          printf("<LOG> Ricevuta una connessione.\n");
          addrlen = sizeof(peer_addr);
          newfd = accept(listener, (struct sockaddr*)&peer_addr, &addrlen);
          FD_SET(newfd, &master);
          if(newfd>fdmax){
            fdmax = newfd;
          }
        }
        else if(i == fileno(stdin)){
          fgets(buffer, BUF_LEN, stdin);
          if(!chatting){
            strtok(buffer, "\n");
            nump = parametrs_num(buffer);
            command = strtok(buffer," ");
            if(!logged && strcmp(command,"signup")==0 && (nump == 3)){
              // ************************************************
              // strtok(NULL ... ) continua a riferirsi
              // all'ultima stringa passata quindi buffer
              // ************************************************
              number = strtok(NULL, " ");
              srv_port = atoi(number);
              username = strtok(NULL, " ");
              password = strtok(NULL, " ");

              memset(&srv_addr, 0, sizeof(srv_addr));
              srv_addr.sin_family = AF_INET;
              inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
              srv_addr.sin_port = htons(srv_port);
              srv_sd = socket(AF_INET, SOCK_STREAM, 0);
              ret = connect(srv_sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

              if(ret<0) perror("Connessione non riuscita");
              else if(signup_protocol_client(srv_sd, username, password)){
                printf("Iscrizione avvenuta con successo!\n");
              }

              close(srv_sd);
            }
            else if(!logged && strcmp(command,"in")==0 && (nump == 3)){
              number = strtok(NULL, " ");
              srv_port = atoi(number);
              username = strtok(NULL, " ");
              password = strtok(NULL, " ");

              memset(&srv_addr, 0, sizeof(srv_addr));
              srv_addr.sin_family = AF_INET;
              inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr);
              srv_addr.sin_port = htons(srv_port);
              srv_sd = socket(AF_INET, SOCK_STREAM, 0);
              ret = connect(srv_sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

              printf("<LOG> Apro una connessione TCP con il SERVER\n");
              if(ret<0){
                perror("Connessione non riuscita");
                //exit(0);
              }
              else if(login_protocol_client(srv_sd, username, password, lst_port)){
                printf("Login avvenuto con successo!\n");
                sprintf(logged_username, "%s", username);
                logged = 1;
                load_chats(&l_chat,logged_username);
              }
              else printf("Username o password errati...\n");
            }
            else if(logged && strcmp(command,"out")==0 && (nump == 0)){
              printf("RICHIESTA DI LOGOUT\n");
              if(logout_protocol_client(srv_sd, logged_username)){
                printf("Logout avvenuto con successo!\n");
                //FD_CLR(listener, &master);
                //close(listener);
                close(srv_sd);
                //listener = 0;
                logged = 0;
                save_chats(l_chat, logged_username);
                delete_l_chat(&l_chat);
              }
              else printf("Logout fallito... \n");
            }
            else if(strcmp(command,"help")==0) display_help_message();
            else if(logged && strcmp(command,"chat")==0 && (nump == 1)){
              dest = strtok(NULL, " ");
              if(is_in_addr_book(dest, logged_username)){
                chatting = 1;
                print_chat(l_chat, dest, logged_username);
                append_user(&chatroom, dest);
              }
              else printf("Utente non in RUBRICA\n");
            }
            else if(logged && strcmp(command, "hanging")==0 && (nump == 0)){
              hanging_protocol_client(srv_sd, logged_username);
            }
            else if(logged && strcmp(command, "show")==0 && (nump == 1)){
              sender = strtok(NULL, " ");
              show_protocol_client(srv_sd, logged_username, sender, &l_chat);
            }
            else if(strcmp(command, "esc")==0){
              printf("Arrivederci\n");
              if(logged) save_chats(l_chat, logged_username);
              close(srv_sd);
              close(listener);
              exit(0);
            }
          }
          else {
            strncpy(cmd, buffer, 5);
            strncpy(sh_cmd, buffer, 2);
            cmd[5] = '\0';
            sh_cmd[2] = '\0';
            if(strcmp(sh_cmd,"\\q")==0){
              chatting = 0;
              user = chatroom;
              while(user!=NULL){
                if(user->cht_sd != 0){
                  leave_chatroom_request_protocol_client(user->cht_sd, logged_username);
                  close(user->cht_sd);
                  user->cht_sd = 0;
                }
                user = user->next;
              }
              delete_chatroom(&chatroom);
            }
            else if(strcmp(sh_cmd,"\\p")==0) print_chatroom(chatroom);
            else if(strcmp(sh_cmd,"\\u")==0) group_protocol_client(srv_sd); // CORREGGI IL PROTOCOLLO
            else if(strcmp(sh_cmd,"\\j")==0){
              user = chatroom;
              // invio la richiesta a tutti i membri della chatroom...
              // nel caso solito di utilizzo, solo ad un menbro.
              while(user!=NULL){
                if(user->cht_sd != 0){
                  join_chatroom_request_protocol_client(user->cht_sd, logged_username, &chatroom);
                }
                user = user->next;
              }
            }
            else if(strcmp(sh_cmd,"\\a")==0) {
              strtok(buffer, " ");
              username = strtok(NULL, "\n");
              // Controllo che l'utente sia online, altrimenti non posso aggiungerlo
              if(is_online(srv_sd, username)){
                user = chatroom;
                // Comunico a tutti gli altri membri della
                // chatroom di aggiungere il nuovo membro
                while(user!=NULL){
                  if(user->cht_sd != 0){
                    add_user_request_protocol_client(user->cht_sd, username);
                  }
                  user = user->next;
                }
                // e lo aggiungo
                append_user(&chatroom, username);
              }
              else printf("Utente non ONLINE\n");
            }
            else if(strcmp(cmd, "share")==0){
              strtok(buffer, " ");
              filename = strtok(NULL, " ");
              // Invio separatamente, ad ogni utente il file
              // per non rallentare troppo, potrei creare dei sottoprocessi
              // che inviano
              user = chatroom;
              while(user!=NULL){
                if(user->cht_sd != 0){
                  send_file_protocol_client(&user->addr, filename);
                }
                user = user->next;
              }
            }
            else{
              user = chatroom;
              strcpy(msg_b, buffer);
              while(user!=NULL){
                if(user->chat == NULL){
                  user->chat = find_chat(&l_chat, user->name);
                }
                // Se ho aperto una socket con l'utente destinatario, mando il messaggio
                // altrimenti contact il server per aprire una nuova comunicazione
                if(user->cht_sd != 0){
                  send_msg(user->cht_sd, logged_username, msg_b, user->chat->next_seq_n);
                }
                else {
                  user->cht_sd = new_chat_protocol_client(srv_sd, logged_username, user->name, &user->addr, msg_b, user->chat->next_seq_n);
                }

                msg = create_my_msg(user->name, logged_username, msg_b, user->chat->next_seq_n);

                //add_msg(&l_chat, msg, logged_username);
                push_msg(&(user->chat->l_msg), msg);
                //incremento il numero sequenziale del messaggio
                user->chat->next_seq_n++;
                user = user->next;
              }
            }
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
          //RICHIESTE DAI PEER O DAL SERVER
          // MSG : avvio la procedura di ricezione di un messaggio
          // MAK : avvio il protocollo di ricezione di un ack di ricezione
          // ADD : avvio il protocollo per l'aggiunta di un utente alla chatroom
          // SHR : avvio il protocollo di ricezione di un file
          // BEY : avvio il protocollo di rimozione di un utente che ha abbamdonato la chatroom
          // JNG : avvio il protocollo di sincronizzazione della chatroom

          // le comunicazioni su pipe avvengono con le stesse modalità delle socket:
          // per passare un stringa, prima mando la lunghezza, poi il buffer dei caratteri
          if(strcmp(buffer, "MSG")==0){
            printf("<LOG> Ricevo richiesta di MESSAGE\n");
            recv_msg(srv_sd, i, chatting, logged_username, &l_chat, &chatroom);
          }
          else if(strcmp(buffer, "MAK")==0){
            printf("<LOG> Ricevo richiesta di MESSAGE ACK\n");
            recv_msg_ack_protocol_client(i, &l_chat);
          }
          else if(strcmp(buffer, "ADD")==0){
            printf("<LOG> Ricevo richiesta di ADD USER\n");
            add_user_protocol_client(i, &chatroom, chatting, logged_username);
          }
          else if(strcmp(buffer, "SHR")==0){
            printf("<LOG> Ricevo richiesta di SHARE FILE\n");
            receive_file_protocol_client(i);
          }
          else if(strcmp(buffer, "BEY")==0){
            printf("<LOG> Ricevo richiesta di LEAVE\n");
            leave_chatroom_protocol_client(i, &chatroom, chatting);
          }
          else if(strcmp(buffer, "JNG")==0){
            printf("<LOG> Ricevo richiesta di JOIN (SINCRONIZZAZIONE)\n");
            join_chatroom_protocol_client(i, &chatroom, chatting);
          }
        }
      }
    }
  }

  //save_chats(l_chat, logged_username);
  //close(srv_sd);
  return 0;
}
