#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct movie {
  char *title;
  int year;
  char languages[5][21]; //max number of languages is 5, max length is 20
  double rating;
  struct movie *next;
};

/*
Parse string of languages and insert into movie's language array
*/
void parseLanguages(struct movie *curMovie, char *languageString) {

  char *token = strtok(languageString, ";]");
  int counter = 0;
  //printf("Parsing: \n");
  while(token != NULL) {
    //printf("Second language: %s\n", token);
    strcpy(curMovie->languages[counter], token);
    token = strtok(NULL, ";]");
    counter++;
  }
}

/*
Parse current line which is ',' delimited to create a movie struct with the data in this line
*/
struct movie *createMovie(char *currLine) {
  struct movie *curMovie = malloc(sizeof(struct movie));

  // for use with strtok_r
  char *saveptr;

  // title token
  char *token = strtok_r(currLine, ",", &saveptr);
  curMovie->title = calloc(strlen(token)+1, sizeof(char));
  strcpy(curMovie->title, token);

  // year token
  token = strtok_r(NULL, "[", &saveptr);
  char *year_string = calloc(strlen(token)+1, sizeof(char));
  strcpy(year_string, token);
  curMovie->year = atoi(year_string); // must change string into int, use atoi
  free(year_string);

  // languages tokens
  token = strtok_r(NULL, ",", &saveptr);
  parseLanguages(curMovie, token);

  // rating token
  char *ptr;
  token = strtok_r(NULL, "\n", &saveptr);
  char *rating_string = calloc(strlen(token)+1, sizeof(char));
  strcpy(rating_string, token);
  curMovie->rating = strtod(rating_string, &ptr); // must change string into double, use strtod
  free(rating_string);

  // set next node to NULL in newly created movie entry
  curMovie->next = NULL;

  return curMovie;
}

/*
Return a linked list of movies by parsing data from each line of the specified file.
*/
struct movie *processFile(char *filepath, int *movieCount) {

  FILE *movieFile = fopen(filepath, "r");

  char *curLine = NULL;
  size_t len = 0;
  ssize_t nread;
  char *token;

  // head of linked list
  struct movie *head = NULL;

  // tail of linked list
  struct movie *tail = NULL;

  // get past the first line since there is no useful data in it
  getline(&curLine, &len, movieFile);

  while((nread = getline(&curLine, &len, movieFile)) != -1) {

    // get new movie node corresponding to current line
    struct movie *newNode = createMovie(curLine);

    //increment movie counter
    (*movieCount)++;

    // is the linked list empty?
    if(head == NULL) {
      // this node is the first node in the linked list, set head and tail to this node
      head = newNode;
      tail = newNode;
    }

    else {
      // linked list isn't empty, insert into end of list
      tail->next = newNode;
      tail = newNode;
    }

  }

  free(curLine);
  fclose(movieFile);
  return head;
}

/*
  Print all movies in a given year
*/
void moviesInYear(struct movie *curMovie, int s_year) {
  int found = 0;

  // look through all movies
  while(curMovie != NULL) {
    if(curMovie->year == s_year) {
      printf("%s\n", curMovie->title);
      found = 1;
    }

    curMovie = curMovie->next;
  }

  // if the language is not found
  if(found == 0) {
    printf("No movies found in %d", s_year);
  }
}

/*
  Find the best rated movie in a year
*/
void bestMovieInYear(struct movie *curMovie, int s_year) {
  int found = 0;
  double top_rating = 0;
  struct movie *topMovie;

  // look through all movies
  while(curMovie != NULL) {

    // make sure that movie is in year we're searching for
    if(curMovie->year == s_year) {
      found = 1;
      if(curMovie->rating > top_rating) {
        top_rating = curMovie->rating;
        topMovie = curMovie;
      }
    }

    curMovie = curMovie->next;
  }

  // if a movie is found for this year, print
  if(found == 1) {
    printf("%d %.1f %s\n", topMovie->year, topMovie->rating, topMovie->title);
  }
}

/*
  Print the best rated movies for each year
*/
void bestRatings(struct movie *curMovie) {
  int oldest = curMovie->year;
  int newest = curMovie->year;

  struct movie *temp = curMovie;

  // calculate the oldest and
  while(temp != NULL) {
    if(temp->year < oldest) {
      oldest = temp->year;
    }

    else if(temp->year > newest) {
      newest = temp->year;
    }

    temp = temp->next;
  }

  for(int i = oldest; i <= newest; i++) {
    bestMovieInYear(curMovie, i);
  }

}

/*
  Print all movies with given language
*/
void moviesInLanguage(struct movie *curMovie, char *language) {
  int found = 0;

  // look through all movies
  while(curMovie != NULL) {

    // look through all languages of a movie
    for(int i = 0; i<5; i++) {
      int ret = strcmp(language, curMovie->languages[i]);
      if(ret == 0) {
        printf("%d %s\n", curMovie->year, curMovie->title);
        found = 1;
        break;
      }
    }
    curMovie = curMovie->next;
  }

  // if the language is not found
  if(found == 0) {
    printf("No movies found in %s\n", language);
  }

}

/*
  Print all attributes of a movie
*/
void printMovie(struct movie *curMovie) {
  printf("%s, %d, %.1f\n", curMovie->title, curMovie->year, curMovie->rating);
  for(int i=0; i<5; i++) {
    printf("%s ", curMovie->languages[i]);
  }
  printf("\n");
  printf("\n");
}

/*
   Print entire list of movies
*/
void printMovieList(struct movie *movieList) {
  while(movieList != NULL) {
    printMovie(movieList);
    movieList = movieList->next;
  }
}

/*
   Frees memory taken up by all movies
*/
void freeList(struct movie *curMovie) {
  struct movie *temp;
  while(curMovie != NULL) {
    temp = curMovie;
    curMovie = curMovie->next;
    free(temp->title);
    free(temp);
  }
}

int main(int argc, char *argv[]) {

  if(argc < 2) {
    printf("You must provide the name of the file to process\n");
    printf("Example usage: ./students student_info1.txt\n");
    return EXIT_FAILURE;
  }

  int movieCount = 0;
  int *ptr = &movieCount;
  struct movie *list = processFile(argv[1], ptr);

  printf("Processed file %s and parsed data for %d movies.\n", argv[1], *ptr);
  int repeat = 1;

  do {
    int choice = 0;
    printf("\n1. Show movies released in the specified year\n");
    printf("2. Show highest rated movie for each year\n");
    printf("3. Show the title and year of release of all movies in a specific language\n");
    printf("4. Exit from the program\n");
    printf("\n");
    printf("Enter a choice from 1 to 4: ");
    scanf("%d", &choice);

    if(choice == 1) {
      int year = 0;
      printf("Enter the year for which you want to see movies: ");
      scanf("%d", &year);
      moviesInYear(list, year);
    }

    else if(choice == 2) {
      bestRatings(list);
    }

    else if(choice == 3) {
      char language[20];
      printf("Enter the language for which you want to see movies: ");
      scanf("%s", language);
      moviesInLanguage(list, language);

    }

    else if(choice == 4) {
      repeat = 0;
    }

    else {
      printf("You entered an incorrect choice. Try again.\n\n");
      repeat = 1;
    }


  } while(repeat == 1);

  //printMovieList(list);
  freeList(list);
  return EXIT_SUCCESS;
}
