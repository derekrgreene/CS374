#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void keygen(int keylength) {
  char allowedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  
  for (int i = 0; i < keylength; i++) {
    printf("%c", allowedChars[rand() % 27]);
  }
  printf("\n");
}


int main(int argc, char *argv[]) {
  int keylength = atoi(argv[1]);
  srand(time(NULL));
  keygen(keylength);
} 