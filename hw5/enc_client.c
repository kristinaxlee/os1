#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// encrypt and decrypt code: https://repl.it/@KristinaLee/FatalHonoredNasm#main.c

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(0);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in *address, int portNumber, char *hostname) {

  // Clear out the address struct
  memset((char *)address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent *hostInfo = gethostbyname(hostname);

  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(0);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char *)&address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);

}

/*
  Checks input file for bad input. Prints an error if the characters aren't A - Z or ' '
*/
void checkBadInput(char *buffer, int fileSize) {

  for(int i=0; i<fileSize; i++) {
    if (buffer[i] != ' ' && buffer[i] != '\0'){
      if (buffer[i] > 'Z' || buffer[i] < 'A'){
        fprintf(stderr, "%s", "enc_client error: input contains bad characters\n");
        exit(1);
      }
    }
  }

}

/* 
  Gets the contents of the passed file and inserts the contents into the buffer string.
*/
void getFile(char *filename, char *buffer) {
  FILE *file = fopen(filename, "r");

  if(file == NULL) {
    fprintf(stderr, "can't open file\n");
  }

  else {
    
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(buffer, 1, fileSize, file);
    buffer[fileSize-1] = '\0'; // change the /n character to /0, MAY BE A PROBLEM HERE?? must check in the future when debugging
    fclose(file);
    
    checkBadInput(buffer, fileSize);

  }
  
}

int main(int argc, char *argv[]) {

  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[200000];

  char plaintext[100000];
  char key[100000];

  // Check usage & args
  if (argc < 4) {
    fprintf(stderr, "USAGE: %s plaintext key port\n", argv[0]);
    exit(0);
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0) {
    error("CLIENT: ERROR opening socket");
  }

  // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost"); //CHANGING HERE!!

  // Connect to server
  if (connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    fprintf(stderr, "Error: could not contact dec_server on port %s\n", argv[3]);
    exit(2);
  }

  
  // Load plaintext into the plaintext string
  getFile(argv[1], plaintext);

  // load the key into the key string
  getFile(argv[2], key);

  // check and see if the key is big enough
  if(strlen(key) < strlen(plaintext)) {
    fprintf(stderr, "Error: key %s is too short\n", argv[2]);
    exit(1);
  }

  strcat(buffer, "e_"); // add e in front of the message and key so that the server knows we're encrypting
  strcat(buffer, plaintext);
  strcat(buffer, "&"); // put & in between the plaintext and the key
  strcat(buffer, key);
  strcat(buffer, "."); // put . to denote the end of the key

  // Send message to server
  // Write to the server
  charsWritten = send(socketFD, buffer, strlen(buffer), 0); // so far only sends the plaintext, need to send the key next

  if (charsWritten < 0) {
    error("CLIENT: ERROR writing to socket");
  }

  if (charsWritten < strlen(buffer)) {
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));

  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0) {
    error("CLIENT: ERROR reading from socket");
  }

  //printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
  printf("%s\n", buffer);

  // Close the socket
  close(socketFD);
  return 0;
}