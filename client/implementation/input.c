#include "../input.h"

int parametrs_num(char* str){
  int count = 0;
  char *ptr = str;
  while((ptr = strchr(ptr, ' ')) != NULL) {
      count++;
      ptr++;
  }
  return count;
}

/*

char* tmp = str;
char* token;
int i = 1;
token = strtok(tmp," ");
do{
  printf("%s\n", token);
  token = strtok(NULL," ");
  i++;
} while(token!=NULL);
return i;

int count = 0;
char *ptr = s;
while((ptr = strchr(ptr, ' ')) != NULL) {
    count++;
    ptr++;
}

*/
