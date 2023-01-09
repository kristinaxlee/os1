#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


/* 
  Creates a random key with a specified length. Currently, the largest key size we can make is 100000
*/
void keygen(int length) {
  // create key and set all chars to be null
  char key[100000];
  memset(key, '\0', 100000);

  srand(time(NULL));

  for (int i = 0; i < length; i++) {
    int x = rand() % 27;
    char letter;

    if (x == 26) {
      letter = ' ';
    }

    else {
      letter = 'A' + (rand() % 26);
    }

    key[i] = letter;
  }

  printf("%s\n", key);
}

int main(int argc, char **argv) {
  keygen(atoi(argv[1]));
}