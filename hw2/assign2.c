#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define PREFIX "movies_"

struct movie {
  char *title;
  int year;
  char languages[5][21]; //max number of languages is 5, max length is 20
  double rating;
  struct movie *next;
};

/*
  Generates a directory name with a random number attached.
  This function allocates memory, so it must be freed later.
*/
char *createDirName() {

  const char *name = "leekr.movies.";

  srand(time(NULL));
  int randomNum = random() % 100000;

  // change our int into string form
  char number[10];
  sprintf(number, "%d", randomNum);

  char *fileName = malloc(strlen(name)+strlen(number)+1);

  // copy both prefix and number into a single string
  strcpy(fileName, name);
  strcat(fileName, number);

  return fileName;
}

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
struct movie *processFile(char *filepath) {

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
  Gets all movies for a specific year and appends to YYYY.txt file
*/
void createMovieFile(char *dirPath, struct movie *curMovie, int year) {

  // open a file for read or write, creating it if necessary and truncating if it already exists
  int fd;

  /*char *newFilePath = createFileName(year);
  char *fullPath = malloc(strlen(newFilePath)+strlen(dirPath)+2);

  strcpy(fullPath, dirPath);
  strcat(fullPath, "/");
  strcat(fullPath, newFilePath);*/

  char fullPath[50];
  const char *extension = ".txt";
  const char *slash = "/";
  sprintf(fullPath, "%s%s%d%s", dirPath, slash, year, extension);

  // We first open a file for read write, creating it if necessary and appending to it if it already exists
  fd = open(fullPath, O_RDWR | O_CREAT | O_APPEND, 0640);
  if (fd == -1){
  	printf("open() failed on \"%s\"\n", fullPath);
  	perror("Error");
  	exit(1);
  }

  // Append new line to title
  char *title = malloc(strlen(curMovie->title)+2);
  strcpy(title, curMovie->title);
  strcat(title, "\n");

  // We write a string to the file
  write(fd, title, strlen(curMovie->title) + 2);

  // Close the file descriptor
  close(fd);

  // Free memory
  free(title);

}

/*
  Goes through all of the movies and creates a YYYY.txt file for each year a movie was made,
  and appends all of the movies in a year to that file.
*/
void processMovies(char *dirName, struct movie *curMovie) {

  // look through all movies and add their titles into the respective year txt files
  while(curMovie != NULL) {

    createMovieFile(dirName, curMovie, curMovie->year);

    curMovie = curMovie->next;

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
    free(temp->title); //must free title since extra memory was allocated for it
    free(temp);
  }

}

/*
   Creates a directory and returns the name of the new directory.
*/
char *createDir() {

  /*char dirName[50];
  const char *name = "leekr.movies.";
  int randomNum = random() % 100000;

  sprintf(dirName, "%s%d", name, randomNum);*/

  char *dirName = createDirName();
  int status = mkdir(dirName, 0750);
  if(status == 0) {
    printf("Directory %s successfully created.\n", dirName);
  }

  return dirName;
}

/*
  Calls all functions necessary to process a csv file, such as processing the movie csv file, creating a new directory, processing movie text files
  Frees all memory used.
*/
void runOperations(char *entryName) {

  printf("Now processing file: %s\n", entryName);

  // Create list to hold all movies
  struct movie *list = processFile(entryName);

  // Create a new directory
  char *dirName = createDir();

  // Populate directory with year.txt files (currently only does it for the first movie)
  processMovies(dirName, list);

  // Free list memory
  freeList(list);
  free(dirName);

}

/*
  Prints the name of the file or directory in the current directory that has the largest file size with the prefix movie.
  Restructuring code from the "Getting File and Directory Meta-Data" example code in the Exploration: Directories module page
*/
void largestFile() {

  // Open the current directory
  DIR* currDir = opendir(".");
  struct dirent *aDir;
  off_t largestFileSize;
  struct stat dirStat;
  int i = 0;
  char entryName[256];

  // Go through all the entries
  while((aDir = readdir(currDir)) != NULL){

    if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){
      // Get meta-data for the current entry
      stat(aDir->d_name, &dirStat);

      // Use the difftime function to get the time difference between the current value of lastModifTime and the st_mtime value of the directory entry
      if(i == 0 || dirStat.st_size > largestFileSize){
        largestFileSize = dirStat.st_size;
        memset(entryName, '\0', sizeof(entryName));
        strcpy(entryName, aDir->d_name);
      }
      i++;
    }
  }

  runOperations(entryName);
  // Close the main directory
  closedir(currDir);
}

/*
  Prints the name of the file or directory in the current directory that has the largest file size with the prefix movie.
  Restructuring code from the "Getting File and Directory Meta-Data" example code in the Exploration: Directories module page
*/
void smallestFile() {
  // Open the current directory
  DIR* currDir = opendir(".");
  struct dirent *aDir;
  off_t smallestFileSize;
  struct stat dirStat;
  int i = 0;
  char entryName[256];

  // Go through all the entries
  while((aDir = readdir(currDir)) != NULL){

    if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0){
      // Get meta-data for the current entry
      stat(aDir->d_name, &dirStat);

      // Use the difftime function to get the time difference between the current value of lastModifTime and the st_mtime value of the directory entry
      if(i == 0 || dirStat.st_size < smallestFileSize){
        smallestFileSize = dirStat.st_size;
        memset(entryName, '\0', sizeof(entryName));
        strcpy(entryName, aDir->d_name);
      }
      i++;
    }
  }

  runOperations(entryName);
  // Close the main directory
  closedir(currDir);

}

/*
  Prints the name of the file or directory in the current directory that has the largest file size with the prefix movie.
  Restructuring code from the "Getting File and Directory Meta-Data" example code in the Exploration: Directories module page
*/
void nameFile(char *filename) {
  // Open the current directory
  DIR* currDir = opendir(".");
  struct dirent *aDir;
  struct stat dirStat;
  int i = 0;
  char entryName[256];

  int found = 0;

  // Go through all the entries
  while((aDir = readdir(currDir)) != NULL){
    if((strcmp(aDir->d_name, filename)) == 0) {
      memset(entryName, '\0', sizeof(entryName));
      strcpy(entryName, aDir->d_name);
      found = 1;
    }
  }

  if(found == 0) {
    printf("File not found, please try again.\n");
    closedir(currDir);
    return;
  }

  else {
    runOperations(entryName);
    // Close the main directory
    closedir(currDir);
  }

}

void menu() {

  int choice = 0;

  printf("Which file you want to process?\n");
  printf("Enter 1 to pick the largest file\n");
  printf("Enter 2 to pick the smallest file\n");
  printf("Enter 3 to specify the name of a file\n");
  printf("Enter a choice from 1 to 3: ");

  scanf("%d", &choice);

  // largest file
  if(choice == 1) {
    largestFile();
  }

  // smallest file
  else if(choice == 2) {
    smallestFile();
  }

  // specify name of file
  else if(choice == 3) {
    char filename[256];
    printf("Enter the full name of the file you want to parse: ");
    scanf("%s", filename);

    nameFile(filename);
  }

  // invalid choice
  else {
    printf("Invalid choice, please try again.\n");
    return;
  }

}

int main() {

  int runAgain = 0;

  do {

    int choice = 0;
    printf("1. Select file to process\n");
    printf("2. Exit the program\n");
    printf("Enter a choice: ");
    scanf("%d", &choice);

    if(choice == 1) {
      menu();
      runAgain = 1;
    }

    else if(choice == 2) {
      runAgain = 0;
    }

    else {
      printf("Invalid choice. Please try again.\n");
      runAgain = 1;
    }

  } while(runAgain == 1);

  return 0;
}
