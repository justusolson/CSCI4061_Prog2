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
  int number = makeargv(string, ",", &arguments);
  int i;
  for(i=0; i<number; i++){
    makeargv(arguments[0], ":", &cnv);
    voteCount = atoi(cnv[1]);
    if(voteCount>max){
      max = voteCount;
      strcpy(winnerName, cnv[0]);
    }
    free(cnv);
    free(*cnv);
  }
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

returns the character string of the output file path
************************************************************************/
char* executeCount(char* path){
  pid_t pid = fork();		//fork for every child node

  if(pid==0){//child node
    char* program = malloc(1024);
    char** args;
    sprintf(program, "./Aggregate_Votes %s", path);
    makeargv(program, " ", &args);
    free(program);//deallocate memory
    execvp(args[0], args);	//exec
    perror("ERROR DOING EXEC: \n");
  }
  else if(pid>0){//parent node
    wait(NULL);
    char* votesFile = malloc(1024*sizeof(char));
    char** args;
    int n = makeargv(path, "/", &args);
    sprintf(votesFile,"%s.txt", args[n-1]);//THIS DOESN'T WORK QUITE RIGHT. IF PATH IS /Documents/Who_Won, THEN OUTPUTFILE WILL BE /Documents/Who_Won.txt
    printf("votesfile: %s\n", votesFile);
    free(*args);
    free(args);
    DIR* direntStream = opendir(path);
    struct dirent* thisDir;
    FILE *read, *write;

    while(1)
    {
      thisDir = readdir(direntStream);
      printf("hello %s\n", thisDir->d_name);
      if(thisDir==NULL){ //end of directory
        break;
      }
      else if(thisDir->d_type == DT_REG && strcmp(thisDir->d_name, votesFile)==0){//if a file is a regular flie and matches output.txt
        //read file to determine who Who_Won
        printf("hello from else if\n");

        int length = strlen(path)+strlen(thisDir->d_name);
  			char* inPath = malloc((length+10)*sizeof(char));	//allocates space for full path name
  			sprintf(inPath, "%s/%s",path,thisDir->d_name);		//creates string for full path
  			read = fopen(inPath, "r");

        char* temp = malloc(MAX_CANDIDADTES*1024*sizeof(char));
  			size_t length2;
  			getline(&temp, &length2, read);

        char* winner = parseWinner(temp); //parseWinner function will take data string and return the string containing the winner's candidateName
        free(temp);
        fclose(read);
        write = fopen(inPath, "a");
        printf("Winner %s\n", winner);
        char* outputString = (char*)malloc(sizeof(char)*(strlen(winner)+10));
        sprintf(outputString, "\nWinner: %s\n", winner);
        free(winner);
        fputs(outputString, write);
        fclose(write);
        free (outputString);
        free(votesFile);

        return inPath;
      }
    }
    return;
  }
  else{
    perror("ERROR FORKING:\n");
    return;
  }
  return;
}



int main(int argc, char** argv){
  if(argc != 2){ //checks for proper number of arguments
    printf("Wrong number of args to Vote_Counter, expected 1, given %d\n", argc - 1);
		exit(1);
  }
  else {
    int len = strlen(argv[1]);
    if(argv[1][len-1] == '/'){
      argv[1][len-1] = 0;
    }
    char* filename = executeCount(argv[1]);
    printf("%s\n", filename);
    free(filename);
  }

  return 0;
}
