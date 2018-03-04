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
    char* outputFile = malloc(1024*sizeof(char));
    sprintf(outputFile,"%s.txt",path);//THIS DOESN'T WORK QUITE RIGHT. IF PATH IS /Documents/Who_Won, THEN OUTPUTFILE WILL BE /Documents/Who_Won.txt
    DIR* direntStream = opendir(path);
    struct dirent* thisDir;
    while(1)
    {
      thisDir = readdir(direntStream);
      if(thisDir==NULL){ //end of directory
        break;
      }
      else if(thisDir->d_type == DT_REG && strcmp(thisDir->d_name, outputFile)==0){//if a file is a regular flie and matches output.txt
        //read file to determine who Who_Won
        int length = strlen(path)+strlen(thisDir->d_name);
  			char* inPath = malloc((length+10)*sizeof(char));	//allocates space for full path name
  			sprintf(inPath, "%s/%s",path,thisDir->d_name);		//creates string for full path
  			input = fopen(inPath, "r");

        char* temp = malloc(MAX_CANDIDADTES*1024*sizeof(char));
  			size_t length2;
  			getline(&temp, &length2, input);

        char* winner = parseWinner(temp); //parseWinner function will take data string and return the string containing the winner's candidateName


        //write to output file
      }
    }
    return 0;
  }
  else{
    perror("ERROR FORKING:\n");
    return;
  }
  return;
}



int main(int argc, char** argv){
  if(argc > 2 || argc < 1){ //checks for proper number of arguments
    printf("Wrong number of args to Vote_Counter, expected at most 1, given %d\n", argc - 1);
		exit(1);
  }

  if(argc == 2){
    char* filename = executeCount(argv[1]);
  }
  else{
    char* filename = executeCount("");//CONFUSED ABOUT HOW 0 ARGUMENTS IS SUPPOSED TO WORK
  }

  return 0;
}
