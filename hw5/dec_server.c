#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in *address, int portNumber) {

  // Clear out the address struct
  memset((char *)address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int getNumber(char x) {
  if(x == ' ') {
    return 26;
  }
  return x - 65;
}

void decrypt(char *buffer) {

  // get the plaintext
  char *ciphertext = strtok(NULL, "&");

  // get the key
  char *key = strtok(NULL, ".");

  char plaintext[100000];
  memset(plaintext, '\0', 100000);

  // encipher the plaintext
  for (int i = 0; i < strlen(ciphertext); i++) {

    // get the number equivalent to a letter, ex. A = 0, B = 1
    int a = getNumber(ciphertext[i]);
    int b = getNumber(key[i]);
    
    // calculate the decrypted letter
    char x = a - b;
  
    // if it's a negative number, then make it positive
    if(x < 0) {
      x += 27;
    }

    // if the character is a space, put space
    if(x == 26) {
      plaintext[i] = ' ';
    }

    // else, it's a letter
    else {
      x += 65; // add 65 so that it's the ASCII equivalent
      plaintext[i] = x;
    }
  }

  // copy ciphertext into the buffer
  memset(buffer, '\0', 200000);
  strcpy(buffer, plaintext);
}

int main(int argc, char *argv[]) {
  int connectionSocket, charsRead;
  char buffer[200000];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) {
    fprintf(stderr, "USAGE: %s port\n", argv[0]);
    exit(1);
  }

  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5);

  // Accept a connection, blocking if one is not available until one connects
  while (1) {
    
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
    if (connectionSocket < 0) {
      error("ERROR on accept");
    }

    pid_t childPid = fork();
    int childStatus;

    if (childPid < 0) {
      error("Hull breach!\n");
    }
    
    else if(childPid == 0) {

      // Get the message from the client and display it
      memset(buffer, '\0', 200000);

      // Read the client's message from the socket
      charsRead = recv(connectionSocket, buffer, 199999, 0);
      
      if (charsRead < 0) {
        error("ERROR reading from socket");
      }

      // check if we're getting the correct message
      char *signal = strtok(buffer, "_");

      if (strcmp(signal, "d") != 0) {
        write(connectionSocket, "Error, wrong server (decryption)", 32);
        exit(2);
      }

      // decrypt the cipertext using the key
      decrypt(buffer);


      // Send a Success message back to the client
      charsRead = send(connectionSocket, buffer, strlen(buffer), 0);
      if (charsRead < 0) {
        error("ERROR writing to socket");
      }

      // Close the connection socket for this client
      close(connectionSocket);
      exit(0);
    }

    // parent process
    else {
      pid_t returnPid = waitpid(childPid, &childStatus, WNOHANG);
    }
  
  }

  // Close the listening socket
  close(listenSocket);
  return 0;
}
