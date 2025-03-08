#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>

#define BUFFER_SIZE 1024


void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 


void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname) {
  memset((char*) address, '\0', sizeof(*address)); 
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  struct hostent* hostInfo = gethostbyname(hostname); 

  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0],hostInfo->h_length);
}


int validChars(char* fileName) {
  FILE* file = fopen(fileName, "r");
  char c;
  while ((c = fgetc(file)) != EOF) {
    if ((c < 'A' || c > 'Z') && c != ' ' && c != '\n') {
      fclose(file);
      return 0;
    }
  }
  fclose(file);
  return 1;
}


char* processFile(char* fileName, long *fileSize) {
  struct stat fileStat;
  FILE* file = fopen(fileName, "r");

  if (file == NULL) {
    error("CLIENT: ERROR opening file");
  }

  stat(fileName, &fileStat);
  *fileSize = fileStat.st_size;

  if (!validChars(fileName)) {
    fprintf(stderr, "error: input contains bad characters");
    exit(1);
  }

  char* buffer = malloc(*fileSize + 1);
  fread(buffer, 1, *fileSize, file);
  buffer[*fileSize] = '\0';
  fclose(file);
  return buffer;
}


int main(int argc, char *argv[]) {
  int socketFD, windowSize, totalSent, totalReceived, bytesSent, 
  bytesReceived, remainingBytes, messageSize, encryptedSize;
  struct sockaddr_in serverAddress;
  char buffer[BUFFER_SIZE];
  char *plaintext, *key, *encryptedData;
  long plaintextSize, keySize;
  
  if (argc < 4) { 
    fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]); 
    exit(0); 
  } 
  
  plaintext = processFile(argv[1], &plaintextSize);
  key = processFile(argv[2], &keySize);

  if (keySize < plaintextSize) {
    fprintf(stderr, "CLIENT: ERROR key '%s' is too short\n", argv[2]);
    exit(1);
  }
  
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0) {
    error("CLIENT: ERROR opening socket");
  }
  
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
    error("CLIENT: ERROR connecting");
  }

  memset(buffer, '\0', BUFFER_SIZE);
  recv(socketFD, buffer, BUFFER_SIZE - 1, 0);

  if (strcmp(buffer, "dec_server") != 0) {
    fprintf(stderr, "CLIENT: ERROR Invalid Server on port %s", argv[3]);
    close(socketFD);
    exit(2);
  }

  windowSize = htonl((int)plaintextSize);
  if (send(socketFD, &windowSize, sizeof(windowSize), 0) < 0) {
    error("CLIENT: ERROR sending plaintext size");
  }
  
  totalSent = 0;
  while (totalSent < plaintextSize) {
    remainingBytes = plaintextSize - totalSent;
    if (remainingBytes > BUFFER_SIZE) {
        messageSize = BUFFER_SIZE;
    } else {
        messageSize = remainingBytes;
    }
    bytesSent = send(socketFD, plaintext + totalSent, messageSize, 0);
    totalSent += bytesSent;
  }

  memset(buffer, '\0', BUFFER_SIZE);
  if (recv(socketFD, buffer, BUFFER_SIZE - 1, 0) < 0) {
    error("CLIENT: ERROR reading ack from socket");
  }
  windowSize = htonl((int)keySize);

  if (send(socketFD, &windowSize, sizeof(windowSize), 0) < 0) {
    error("CLIENT: ERROR sending key size");
  }
  
  totalSent = 0;
  while (totalSent < keySize) {
    remainingBytes = keySize - totalSent;
    if (remainingBytes > BUFFER_SIZE) {
      messageSize = BUFFER_SIZE;
    } else {
      messageSize = remainingBytes;
    }
    bytesSent = send(socketFD, key + totalSent, messageSize, 0);
    if (bytesSent < 0) {
      error("CLIENT: ERROR writing key to socket");
    }
    totalSent += bytesSent;
  }

  memset(buffer, '\0', BUFFER_SIZE);
  if (recv(socketFD, buffer, BUFFER_SIZE - 1, 0) < 0) {
    error("CLIENT: ERROR reading ack from socket");
  }

  if (recv(socketFD, &encryptedSize, sizeof(encryptedSize), 0) < 0) {
    error("CLIENT: ERROR reading encrypted size from socket");
  }
  encryptedSize = ntohl(encryptedSize);
  encryptedData = malloc(encryptedSize + 1);

  if (encryptedData == NULL) {
    error("CLIENT: ERROR allocating memory for encrypted data");
  }
  memset(encryptedData, '\0', encryptedSize + 1);
  
  totalReceived = 0;
  while (totalReceived < encryptedSize) { 
    bytesReceived = recv(socketFD, encryptedData + totalReceived, encryptedSize - totalReceived, 0);
    
    if (bytesReceived < 0) {
      error("CLIENT: ERROR reading encrypted data from socket");
    }
    if (bytesReceived == 0) {
      break; 
    }
    totalReceived += bytesReceived;
  }
  
  printf("%s", encryptedData);
  free(plaintext);
  free(key);
  free(encryptedData);
  close(socketFD);
  return 0;
}