#define NUM_ARGS 1
#define MAX_CANDIDADTES 100

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "makeargv.h"

char* parseWinner(char* string);
int executeCount(char* path);
int containsCycle(char* path);

/************************************************************************
Function: parseWinner
Description: Function takes string of vote data and parses the string to
            it parses this string to determine the candidate name of the
            winner.
            input: "A:3,B:1,C:1,D:1,E:4"
            output: "E"

returns the character string of the winner's candidate name
************************************************************************/
char* parseWinner(char* string){
  int max =0;
  char* winnerName = (char*)malloc(1024*sizeof(char));
  int voteCount;
  char** arguments;
  char** cnv;
  int number = makeargv(string, ",", &arguments); //split results by comments, each element of arguments contains "candidate:votes"
  int i;
  for(i=0; i<number; i++){
    makeargv(arguments[i], ":", &cnv); //split arguments by colon, cnv[0] is name and cnv[1] is number of votes
    voteCount = atoi(cnv[1]);

    if(voteCount>max){ // find name of candidate with max number of votes
      max = voteCount;
      strcpy(winnerName, cnv[0]);
    }
    free(*cnv);
    free(cnv);
  }
  free(*arguments);
  free(arguments);
  return winnerName;
}

/************************************************************************
Function: executeCount
Description: Function takes a path as an argument. The path is the root
            directory of the voting tree. It will create a child process
            to call Aggregate_Votes on this path. It will read the output
            txt file to determine the winner and append the winner's name
            to the output txt file.
            Ex: path: Who_Won
                output: Winner:E

returns 1 if succeeded, -1 if failed.
************************************************************************/
int executeCount(char* path){
  pid_t pid = fork();		//fork to exec Aggregate_Votes

  if(pid==0){//child process
    char* program = malloc(1024);
    char** args;
    sprintf(program, "./Aggregate_Votes %s", path);
    makeargv(program, " ", &args);
    free(program);//deallocate memory
    execvp(args[0], args);	//exec
    perror("ERROR DOING EXEC: \n");
  }
  else if(pid>0){//parent process
    wait(NULL);
    char* votesFile = malloc(1024*sizeof(char));
    char** args;
    int n = makeargv(path, "/", &args);
    sprintf(votesFile,"%s.txt", args[n-1]); //votes file is final output file produced by Aggregate_Votes
    free(*args);
    free(args);
    DIR* direntStream = opendir(path);
    struct dirent* thisDir;
    FILE *read, *write;

    while(1)
    {
      thisDir = readdir(direntStream); // next file in directory
      if(thisDir==NULL){ //end of directory
        break;
      }
      else if(thisDir->d_type == DT_REG && strcmp(thisDir->d_name, votesFile)==0){//if a file is a regular flie and matches output.txt
        //read file to determine who Who_Won
        int length = strlen(path)+strlen(thisDir->d_name);
  			char* inPath = malloc((length+10)*sizeof(char));	//allocates space for full path name
  			sprintf(inPath, "%s/%s",path,thisDir->d_name);		//creates string for full path
  			read = fopen(inPath, "r");

        char* temp = malloc(MAX_CANDIDADTES*1024*sizeof(char));
  			size_t length2;
  			getline(&temp, &length2, read); // read line from file
        char* winner = parseWinner(temp); //parseWinner function will take data string and return the string containing the winner's candidateName
        free(temp);
        fclose(read);
        write = fopen(inPath, "a");
        char* outputString = (char*)malloc(sizeof(char)*(strlen(winner)+10));
        sprintf(outputString, "Winner:%s\n", winner);
        free(winner);
        fputs(outputString, write); // write winner to output file
        fclose(write);
        free (outputString);
        free(votesFile);
        free(inPath);
        return 1;
      }
    }
    return -1;
  }
  else{
    perror("ERROR FORKING:\n");
    return;
  }
  return;
}

int containsCycle(char* path){
  char** visited;
  char** fringe;
  int cycle = 0;
  DIR* curdir = malloc(sizeof(DIR*));
  struct dirent* thisdir = malloc(sizeof(struct dirent*));
  while(1){
    curdir = opendir(path);
    if(curdir == NULL){
      printf("Failed to open path.\n");
      break;
    }
    while(1){
      thisdir = readdir(curdir);
      if(thisdir == NULL){
        break;
      }
      if(strcmp(thisdir->d_name, ".")!=0 && strcmp(thisdir->d_name, "..")!=0){
  			if(thisdir->d_type == DT_DIR){
          cycle = 1;
        }
      }
    }
    //update path to
  }
  free(curdir);
  if(cycle) return 1;
  else return 0;
}

int main(int argc, char** argv){
  if(argc < NUM_ARGS+1){ //checks for proper number of arguments
    printf("Wrong number of args to Vote_Counter, expected 1, given %d\n", argc - 1);
		exit(1);
  }
  else {
    int len = strlen(argv[1]);
    if(argv[1][len-1] == '/'){ // remove slash on end of path if it exists
      argv[1][len-1] = 0;
    }
    int e = executeCount(argv[1]);
    if(e == -1){
      printf("executeCount failed.\n");
      exit(1);
    }
    remove("trash.txt");
  }

  return 0;
}
