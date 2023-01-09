#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_LENGTH 1000
#define MAX_LINES 50
#define PRINT_LENGTH 80

#define NUM_LINES 49

// got this code from the Repl.it provided on the assignment page: https://repl.it/@cs344/65prodconspipelinec#main.c

// buffer 1, shared resource between input thread and addToString thread
char *buffer1[MAX_LINES];
// number of items in buffer
int count_1 = 0;
// index where input thread will put the next item
int prod_idx_1 = 0;
// index where the output thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;



// buffer 2, shared resource between addToString thread and ^^ thread
char *buffer2[MAX_LINES];

// Number of items in the buffer
int count_2 = 0;
// Index where the square-root thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;



// buffer 3, shared resource between ^^ thread and output thread
char *buffer3[MAX_LINES];

// Number of items in the buffer
int count_3 = 0;
// Index where the square-root thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;



// output buffer, only used by output thread
char outputBuffer[MAX_LENGTH*MAX_LINES];

// signal to indicate STOP
int stopInput = 0;
int stopSpace = 0;
int stopPlus = 0;
int stopOutput = 0;

/*
  Function to replace certain substring occurences with different string
*/
void replace(char *string, char *substr, const char *replacement){

  char new_str[512] = {0};
  const char *tmp = string;
  int insert = 0;
  int substr_len = strlen(substr);
  int repl_len = strlen(replacement);

  while (1) {

    // check and see if substring is present in the string
    const char *occurence = strstr(tmp, substr);

    // if not found, then we can copy the rest of the string
    if (occurence == NULL) {
      strcpy(&new_str[insert], tmp);
      break;
    }

    // substring was found, so copy part before it along with replacement string
    else {
      // copy part before substring
      memcpy(&new_str[insert], tmp, occurence - tmp);
      insert += occurence - tmp;

      // copy replacement string
      memcpy(&new_str[insert], replacement, repl_len);
      insert += repl_len;

      // move along with rest of the string
      tmp = occurence + substr_len;
    }

  }

  // put fixed string back into original string
  strcpy(string, new_str);
}

/*
  Prompts user for input and returns a string
  Allocates memory, needs to be freed later
*/
char *getUserInput() {
  // store user input line
  char *userInput = malloc(MAX_LENGTH * sizeof(char));

  fgets(userInput, MAX_LENGTH, stdin);

  return userInput;
}

/*
  Thread to get user input
  Puts input into buffer 1
*/
void *get_input() {

  char input[MAX_LENGTH];

  while (stopInput != 1){

    char *input = getUserInput();

    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_1);

    // copy input into buffer
    buffer1[prod_idx_1] = strdup(input);

    // Increment the index where the next item will be put
    prod_idx_1++;
    count_1++;

    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);

    // if we encounter a stop, then thread can terminate
    if (strcmp(input, "STOP\n") == 0){
      stopInput = 1;
    }

    // free input string
    free(input);
  }

  return NULL;
}

/*
  Thread to remove all newline chars from input
  Puts processed input into buffer 2
*/
void *removeSpace() {

  while (stopSpace != 1) {

    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_1);

    while (count_1 == 0) {

      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full_1, &mutex_1);
    }

    /* GET INPUT FROM BUFFER */
    char input[MAX_LENGTH];
    strcpy(input, buffer1[con_idx_1]); // get next string from buffer 1

    // Increment the index from which the item will be picked up
    con_idx_1 = con_idx_1 + 1;
    count_1--;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);


    /* REMOVE NEWLINE FROM STRING */
    replace(input, "\n", " ");


    /* PUT INPUT INTO BUFFER 2 */

    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_2);
    // put the string in the buffer
    buffer2[prod_idx_2] = strdup(input);
    // Increment the index where the next item will be put.
    prod_idx_2++;
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_2);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);

    // if we encounter a stop, then thread can terminate
    if (strcmp(input, "STOP ") == 0){
      stopSpace = 1;
    }
  }

  return NULL;
}

/*
  Thread to remove ++ and replace with ^
  Puts processed input into buffer 3
*/
void *removePlus() {
  while (stopPlus != 1){

    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_2);

    while (count_2 == 0){
      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full_2, &mutex_2);
    }


    /* GET INPUT FROM BUFFER */
    char input[MAX_LENGTH];
    strcpy(input, buffer2[con_idx_2]); // get next string from buffer 1

    // Increment the index from which the item will be picked up
    con_idx_2 = con_idx_2 + 1;
    count_2--;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);


    /* REPLACE ++ WITH ^ */
    replace(input, "++", "^");


    /* PUT INPUT INTO BUFFER 2 */
    // Lock the mutex before putting the item in the buffer
    pthread_mutex_lock(&mutex_3);
    // put the string in the buffer
    buffer3[prod_idx_3] = strdup(input);
    // Increment the index where the next item will be put.
    prod_idx_3++;
    count_3++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_3);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);

    // if we encounter a stop, then thread can terminate
    if (strcmp(input, "STOP ") == 0) {
      stopPlus = 1;
    }

  }

  return NULL;
}

/*
  Prints out content of output buffer
*/
void printBuffer(char *buffer) {

  char temp[MAX_LENGTH];

  // while we have 80 characters in buffer
  while (strlen(buffer) >= PRINT_LENGTH) {
    // get the first 80 char of buffer and print it
    char output[PRINT_LENGTH + 1];
    strncpy(output, buffer, PRINT_LENGTH);
    printf("%s\n", output);

    // copy all of buffer after first 80 chars
    strcpy(temp, buffer + PRINT_LENGTH);

    // move remainder of string back into original message
    strcpy(buffer, temp);
  }

}

/*
  Thread to handle all of the output
*/
void *writeOutput() {

  while (stopOutput != 1) {

    // Lock the mutex before checking if the buffer has data
    pthread_mutex_lock(&mutex_3);

    while(count_3 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
      pthread_cond_wait(&full_3, &mutex_3);
    }

    /* GET INPUT FROM BUFFER */
    char input[MAX_LENGTH];
    strcpy(input, buffer3[con_idx_3]); // get next string from buffer 1

    if (strcmp(input, "STOP ") == 0) {
      pthread_mutex_unlock(&mutex_3);
      return NULL;
    }

    // Increment the index from which the item will be picked up
    con_idx_3 = con_idx_3 + 1;
    count_3--;

    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);

    // Add new string into output buffer
    strcat(outputBuffer, input);

    /* PRINT THE OUTPUT */
    printBuffer(outputBuffer);

  }
  return NULL;
}

/*
  Free all allocated arguments
*/
void freeArgs() {
  for (int i = 0; i < prod_idx_1; i++) {
    free(buffer1[i]);
  }

  for (int i = 0; i < prod_idx_2; i++) {
    free(buffer2[i]);
  }

  for (int i = 0; i < prod_idx_3; i++) {
    free(buffer3[i]);
  }
}


int main() {
  srand(time(0));
  pthread_t input_t, removeSpace_t, removePlus_t, output_t;

  // create the threads
  pthread_create(&input_t, NULL, get_input, NULL);
  pthread_create(&removeSpace_t, NULL, removeSpace, NULL);
  pthread_create(&removePlus_t, NULL, removePlus, NULL);
  pthread_create(&output_t, NULL, writeOutput, NULL);

  // wait for threads to terminate
  pthread_join(input_t, NULL);
  pthread_join(removeSpace_t, NULL);
  pthread_join(removePlus_t, NULL);
  pthread_join(output_t, NULL);

  freeArgs();

  return EXIT_SUCCESS;
}
