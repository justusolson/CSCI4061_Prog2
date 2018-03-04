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

void executeCount(char* path){
  pid_t pid = fork();		//fork for every child node

  if(pid==0){//child node

  }
  else if(pid>0){//parent node

  }
  else{
    perror("ERROR FORKING\n");
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
    executeCount(argv[1]);
  }
  else{
    executeCount("");
  }

  return 0;
}
