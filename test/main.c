#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
  char s[] = "Giovanni: Il pana ha messo : i sostegni";
  char* msg;
  char* t1;
  t1 = strtok(s,":");
  //s += strlen(t1)+1;
  msg = &s[strlen(t1)+1];
  printf("%s %s\n",t1,msg );
  return 0;
}
