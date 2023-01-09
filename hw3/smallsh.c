#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LENGTH 2048
#define MAX_ARG 512

int allowBackground = 1;

// compile with: gcc --std=gnu99 -o main main.c

/*
  Parses user input line.
  - sets all of argument array to NULL to clear old commands
  - inserts all arguments into argv array
  - puts number of arguments into numArguments
  - checks for $$ and replaces with the pid
*/
void parseInput(char **argv, char *input, int *numArg, int pid) {

  // clear out old arguments
  for(int i = 0; i < MAX_ARG; i++) {
    argv[i] = NULL;
  }

  (*numArg) = 0;
  char *token = strtok(input, " \n");

  // get all arguments from user input
  while(token != NULL) {

    // $$ handler, replace $$ with pid
    if(token[strlen(token) - 2] == '$' && token[strlen(token)-1] == '$') {
      char newToken[2048] = ""; // new token to hold token without the $$
      char newString[2048] = ""; // new string to hold new token + pid
      strncpy(newToken, token, strlen(token)-2); // copy everything except $$ into new token
      sprintf(newString, "%s%d", newToken, pid); // add the pid onto the new token
      fflush(stdout);
      argv[*numArg] = strdup(newString); // insert new token into argv
    }

    else {
      argv[*numArg] = strdup(token);
    }

    (*numArg)++;

    // go to next token
    token = strtok(NULL, " \n");
  }

}

/*
 Looks through argv array for files.
 If there are files present, insert the names of the files into inputFile and outputFile
*/
void getFiles(char **argv, int *numArg, char *inputFile, char *outputFile) {

  // tells whether or not we have to do file i/o
  int fileStuff = 0;

  // look through entire argv array for i/o files
  for(int i = 0; i < (*numArg); i++) {

    if(strcmp(argv[i], "<") == 0) {
      fileStuff = 1;
      strcpy(inputFile, argv[i+1]); //copy inputFile name into string
    }

    else if(strcmp(argv[i], ">") == 0) {
      fileStuff = 1;
      strcpy(outputFile, argv[i + 1]); //copy outputFile name into string
    }

  }

  // if we have to do file i/o, then remove the files and > and < from the argv array since execvp cant do this
  if (fileStuff == 1){
		for(int j = 1; j < (*numArg); j++) {
      argv[j] = NULL;
    }
		(*numArg) = 1;
	}

}

/*
 Prints the exit status of a child process
*/
void getStatus(int status) {
  if(WIFEXITED(status)) {
    printf("exit value %d\n", WEXITSTATUS(status));
  }
  else {
    printf("terminated by signal %d\n", WTERMSIG(status));
  }
  fflush(stdout);
}

/*
 Catch Ctrl-Z signal
*/
void catchSIGTSTP() {
  if (allowBackground == 1) {
    char *message = "Entering foreground-only mode (& is now ignored)\n";
    write(1, message, 49);
    fflush(stdout);
    allowBackground = 0;
  }

  else {
    char *message = "Exiting foreground-only mode\n";
    write(1, message, 29);
    fflush(stdout);
    allowBackground = 1;
  }
}

/*
  Runs all commands that are not built in
*/
void forkCommands(char **argv, int numArg, int *status, int foregroundProc, char *inputFile, char *outputFile, struct sigaction SIGINT_action) {

  pid_t childPid = fork();
  int childStatus;

  if(childPid == -1) {
    perror("Hull Breach!\n");
    exit(1);
  }

  // CHILD PROCESS
  else if(childPid == 0) {

    if(foregroundProc == 1) {
      SIGINT_action.sa_handler = SIG_DFL;
      sigaction(SIGINT, &SIGINT_action, NULL);
    }

    // if running in background and input or output file is not specified, then open /dev/null
    if(foregroundProc == 0 && allowBackground == 1) {
      // check if input file was entered or not
      if (strcmp(inputFile, "") == 0) {

    		int input = open("/dev/null", O_RDONLY);
    		if (input == -1) {
    			perror("cannot open input file\n");
    			exit(1);
    		}

    		int result = dup2(input, 0);
    		if (result == -1) {
    			perror("cannot dup input file\n");
    			exit(2);
    		}

        // use fcntl so that we can change the properties of a file that is already open and can be used to prevent such sharing
    		fcntl(input, F_SETFD, FD_CLOEXEC);
    	}

      // check if output file was entered or not
    	if (strcmp(outputFile, "") == 0) {

    		int output = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    		if (output == -1) {
    			perror("cannot open output file\n");
    			exit(1);
    		}

    		int result = dup2(output, 1);
    		if (result == -1) {
    			perror("cannot dup output file\n");
    			exit(2);
    		}

        // use fcntl so that we can change the properties of a file that is already open and can be used to prevent such sharing
    		fcntl(output, F_SETFD, FD_CLOEXEC);
     }
    }

    // check if input file was entered or not
    if (strcmp(inputFile, "") != 0) {

      int input = open(inputFile, O_RDONLY);
      if (input == -1) {
        perror("cannot open input file\n");
        exit(1);
      }

      int result = dup2(input, 0);
      if (result == -1) {
        perror("cannot dup input file\n");
        exit(2);
      }

      // use fcntl so that we can change the properties of a file that is already open and can be used to prevent such sharing
      fcntl(input, F_SETFD, FD_CLOEXEC);
    }

    // check if output file was entered or not
    if (strcmp(outputFile, "") != 0) {

      int output = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (output == -1) {
        perror("cannot open output file\n");
        exit(1);
      }

      int result = dup2(output, 1);
      if (result == -1) {
        perror("cannot dup output file\n");
        exit(2);
      }

      // use fcntl so that we can change the properties of a file that is already open and can be used to prevent such sharing
      fcntl(output, F_SETFD, FD_CLOEXEC);
    }

    // execute commands
    execvp(argv[0], argv);

    // if execvp returns, then there was an error
    printf("command not found\n");
    fflush(stdout);
    exit(1);

  }

  // PARENT PROCESS
  else {
    // not in foreground, so use WNOHANG
    if(foregroundProc == 0 && allowBackground == 1) {
      // use a different pid for the return value of wait, because if it's a background command then it will return 0
      pid_t returnPid = waitpid(childPid, status, WNOHANG);
      printf("background pid is: %d\n", childPid);
    }

    // if foreground, parent needs to wait for child here
    else {
      childPid = waitpid(childPid, status, 0);
    }

    fflush(stdout);
  }

  // check for background processes
	while ((childPid = waitpid(-1, status, WNOHANG)) > 0) {
		printf("background pid %d is done: ", childPid);
		getStatus(*status);
		fflush(stdout);
	}
}

/*
  Runs built in commands (cd, exit, status)
*/
void runCommands(char **argv, int numArg, int *status, int *foregroundProc, char *inputFile, char *outputFile, struct sigaction SIGINT_action) {

  char *homeDir = "HOME";

  // if line is empty, just return
  if(numArg == 0) {
    return;
  }

  // check and see if command is background or not
  if(strncmp(argv[numArg - 1], "&", 1) == 0) {
    //printf("Background process\n");
    //fflush(stdout);
    (*foregroundProc) = 0;
    argv[numArg - 1] = NULL;
  }

  // check to see if it's one of the built in commands
  if(strncmp(argv[0], "exit", 4) == 0) {
    printf("exiting\n");
    exit(0);
  }

  else if(strncmp(argv[0], "cd", 2) == 0) {
    if(numArg == 1) {
      chdir(getenv("HOME"));
    }
    else {
      chdir(argv[1]);
    }
  }

  else if(strncmp(argv[0], "status", 6) == 0) {
    getStatus(*status);
  }

  else if(strncmp(argv[0], "#", 1) == 0) {
    return;
  }

  // else if it's not a built in command, we need to fork off a child
  else {
    forkCommands(argv, numArg, status, (*foregroundProc), inputFile, outputFile, SIGINT_action);
  }
}

int main() {

  char *argv[MAX_ARG];
  int foregroundProc = 1;
  int status = 0;
  int numArg = 0;
  int pid = getpid();

  // SIGNAL STUFF

  // initialize SIGINT action to be ignored
  struct sigaction SIGINT_action = {0};
  SIGINT_action.sa_handler = SIG_IGN;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;
  sigaction(SIGINT, &SIGINT_action, NULL);

  // initialize SIGTSTP action
  struct sigaction SIGTSTP_action = {0};
  SIGTSTP_action.sa_handler = catchSIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  // store user command line
  char *userInput = malloc(MAX_LENGTH * sizeof(char));

  // go into shell
  while(1) {

    printf(": ");
    fflush(stdout); // MAKE SURE TO FFLUSH AFTER EACH PRINT!!!
    fgets(userInput, MAX_LENGTH, stdin);
    foregroundProc = 1;

    char inputFile[256] = "";
    char outputFile[256] = "";

    parseInput(argv, userInput, &numArg, pid); // get user input and put into argv array
    getFiles(argv, &numArg, inputFile, outputFile); // check and see if i/o files are specified
    runCommands(argv, numArg, &status, &foregroundProc, inputFile, outputFile, SIGINT_action);
  }

  free(userInput);
  return 0;

}
