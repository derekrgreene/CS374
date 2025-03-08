#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

void encrypt(char* plaintext, int textLength, char* key, int keyLength) {
  int charVal, keyVal, encryptedChar;
  
  for (int i = 0; i < textLength; i++) {
    if (plaintext[i] == '\n') {
      continue;
    }
    if (plaintext[i] == ' ') {
      charVal = 26;
    } else {
      charVal = plaintext[i] - 'A';
    }
    if (key[i] == ' ') {
      keyVal = 26;
    } else {
      keyVal = key[i] - 'A';
    }
    
    encryptedChar = (charVal + keyVal) % 27;
    if (encryptedChar == 26) {
      plaintext[i] = ' ';
    } else {
      plaintext[i] = encryptedChar + 'A';
    }
  }
}


void error(const char *msg) {
  perror(msg);
  exit(1);
} 


void setupAddressStruct(struct sockaddr_in* address, int portNumber) {
  memset((char*) address, '\0', sizeof(*address)); 
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  address->sin_addr.s_addr = INADDR_ANY;
}


int main(int argc, char *argv[]) {
  int connectionSocket, plaintextSize, encryptedSize, bytesReceived, bytesSent, 
  totalReceived, totalSent ,networkSize, remainingBytes, messageSize, keySize; 
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  char *plaintext, *key, *ack, *keyAck, *handshake = "enc_server";
  
  if (argc < 2) { 
    fprintf(stderr, "USAGE: %s port\n", argv[0]); 
    exit(1);
  } 

  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }
  
  setupAddressStruct(&serverAddress, atoi(argv[1]));
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    error("ERROR on binding");
  }
  
  listen(listenSocket, 5); 
  while(1) {
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    
    if (connectionSocket < 0) {
      error("ERROR on accept");
    }
    
    pid_t spawnPid = fork();
    switch(spawnPid) {
      case -1: 
        perror("fork()\n");
        exit(1);
        break;
      
      case 0:
        printf("SERVER: Connected to client running at host %d port %d\n", ntohs(clientAddress.sin_addr.s_addr), ntohs(clientAddress.sin_port));
        
        if (send(connectionSocket, handshake, strlen(handshake), 0) < 0) {
          error("ERROR sending handshake");
        }
        if (recv(connectionSocket, &plaintextSize, sizeof(plaintextSize), 0) < 0) {
          error("ERROR reading plaintext size from socket");
        }

        plaintextSize = ntohl(plaintextSize);
        plaintext = malloc(plaintextSize + 1);

        if (plaintext == NULL) {
          error("ERROR allocating memory for plaintext");
        }
        memset(plaintext, '\0', plaintextSize + 1);
        totalReceived = 0;
        
        while (totalReceived < plaintextSize) {
          bytesReceived = recv(connectionSocket, plaintext + totalReceived, plaintextSize - totalReceived, 0);

          if (bytesReceived < 0) {
            error("ERROR reading plaintext from socket");
          }
          if (bytesReceived == 0) {
            break;
          }
          totalReceived += bytesReceived;
        }

        if (plaintext[plaintextSize - 1] == '\n') {
          plaintext[plaintextSize - 1] = '\0';
          plaintextSize--; 
        }
       
        ack = "I am the server, and I got your plaintext";
        if (send(connectionSocket, ack, strlen(ack), 0) < 0) {
          error("ERROR sending plaintext acknowledgment");
        }
        
        keySize;
        if (recv(connectionSocket, &keySize, sizeof(keySize), 0) < 0) {
          error("ERROR reading key size from socket");
        }

        keySize = ntohl(keySize);
        key = malloc(keySize + 1);

        if (key == NULL) {
          error("ERROR allocating memory for key");
        }
        memset(key, '\0', keySize + 1);
        totalReceived = 0;

        while (totalReceived < keySize) {
          bytesReceived = recv(connectionSocket, key + totalReceived, keySize - totalReceived, 0);

          if (bytesReceived < 0) {
            error("ERROR reading key from socket");
          }
          if (bytesReceived == 0) {
            break;
          }
          totalReceived += bytesReceived;
        }

        keyAck = "I am the server, and I got your key";
        if (send(connectionSocket, keyAck, strlen(keyAck), 0) < 0) {
          error("ERROR sending key acknowledgment");
        }

        printf("SERVER: Plaintext received (%d bytes)\n", plaintextSize);
        printf("SERVER: Key received (%d bytes)\n", keySize);
        encrypt(plaintext, plaintextSize, key, keySize);
        plaintext[plaintextSize] = '\n';
        encryptedSize = plaintextSize + 1;
        networkSize = htonl(encryptedSize); 

        if (send(connectionSocket, &networkSize, sizeof(networkSize), 0) < 0) {
          error("ERROR sending encrypted size");
        }
        
        totalSent = 0;
        while (totalSent < encryptedSize) {
          remainingBytes = encryptedSize - totalSent;
          if (remainingBytes > BUFFER_SIZE) {
            messageSize = BUFFER_SIZE;
          } else {
            messageSize = remainingBytes;
          }
          bytesSent = send(connectionSocket, plaintext + totalSent, messageSize, 0);
          
          if (bytesSent < 0) {
            error("ERROR writing encrypted data to socket");
          }
          totalSent += bytesSent;
        }
        
        printf("SERVER: Encrypted data sent (%d bytes)\n", totalSent);
        free(plaintext);
        free(key);
        close(connectionSocket);
        exit(0);
        break;
      
      default: 
        close(connectionSocket);
        break;
    }
  }
  close(listenSocket);
  return 0;
}
