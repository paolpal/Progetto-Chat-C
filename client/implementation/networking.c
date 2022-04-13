#include "../networking.h"

// ****************************************************
// le funzioni send_all(...) e recv_all(...) assicurano
// che la trasmissione e la ricezione siano complete
// ****************************************************

int send_all(int socket, void *buffer, size_t length, int flags){
  int sent = 0;
  char *ptr = (char*) buffer;
  while (length > 0){
    int i = send(socket, ptr, length, flags);
    if (i < 1) return i;
    ptr += i;
    sent +=i;
    length -= i;
  }
  return sent;
}

int recv_all(int socket, void *buffer, size_t length, int flags){
  int recived = 0;
  char* ptr = (char*) buffer;
  do {
    int i = recv(socket, ptr, length, flags);
    if (i < 1) return i;
    ptr += i;
    recived += i;
    length -= i;
  } while (length > 0);
  return recived;
}
