/* Name: Derek Greene
*  OSU Email: greenede@oregonstate.edu
*  Course: CS 374 Operating Systems I
*  Assignment: OTP
*  Due Date: 3/16/2025
*  Description: This program serves as a key generator for a one-time-pad system. The key is generated using 
*               the 27 allowed characters (A-Z and space). The key is output to stdout and length is passed as a command-line argument.
*
*  References:
*  The Linux Programming Interface: a Linux and UNIX system programming handbook. Kerrisk, M. (2010)
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/*
* Function to generate a pseudo-random key of specified length
* Parameters: int keylength
* Returns: void
*/
void keygen(int keylength) {
  char allowedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; // A-Z and space
  
  // generate key
  for (int i = 0; i < keylength; i++) {
    printf("%c", allowedChars[rand() % 27]); // print random char
  }
  printf("\n"); // add newline char
}


/*
* Main Function
* Parameters: int argc, char *argv[]
* Returns: int
*/
int main(int argc, char *argv[]) {
  int keylength;

   // check for correct argument count
   if (argc < 2) { 
    fprintf(stderr, "USAGE: %s keylength\n", argv[0]); 
    exit(0); 
  } 
  
  keylength = atoi(argv[1]); // convert to int
  srand(time(NULL));  // seed random number generator
  keygen(keylength); // generate key
  return 0;
} 