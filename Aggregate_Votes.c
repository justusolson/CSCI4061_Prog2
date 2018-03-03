#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

int isLeaf(char* path);
void aggregateVotes(char* path);

/************************************************************************
Function: isLeaf
Description: determines if a directory contains another directory,
  making it not a leaf, or not.
input: Path to directory
output: 1 if is a leaf, 0 if not a leaf
************************************************************************/
int isLeaf(char* path){
  DIR* curdir = opendir(path);
  struct dirent* thisdir;
  int leaf = 1;
  while(1){
    thisdir = readdir(curdir);
    if(thisdir == NULL){
      break;
    }
    if(strcmp(thisdir->d_name, ".")!=0 && strcmp(thisdir->d_name, "..")!=0){
			if(thisdir->d_type == DT_DIR){
        leaf = 0;
      }
    }
  }
  if(leaf) return 1;
  else return 0;
}

/************************************************************************
Function: aggregateVotes
Description: function recurses through directories forking and execing
  Leaf_Counter on leaves and Aggregate_Votes on non-leaves. Ignores
  votes.txt files in non-leaves.
input: Path to directory
output: Aggregation results in "<path>.txt" files and writes the output
  file path to stdout
************************************************************************/
void aggregateVotes(char* path){
  if(isLeaf(path)){

  }

  DIR* curdir = opendir(path);
  struct dirent* thisdir;

  // curdir = opendir(path);
  while(1){
    thisdir = readdir(curdir);
    if(strcmp(thisdir->d_name, ".")!=0 && strcmp(thisdir->d_name, "..")!=0){
      if(strcmp(thisdir->d_name, "votes.txt")==0){

      }
    }
  }
}

int main(int argc, char** argv)
{
  if(argc < 2){
    printf("Incorrect number of arguments, expected 1 given %d.\n", argc-1);
    return -1;
  }
  if(isLeaf(argv[1])){
    printf("isLeaf\n");
    char* votespath = malloc(strlen(argv[1])+12);
    int len = strlen(argv[1]);
    // printf("last char of argv[1]: %c\n", argv[1][len-1]);
    if(argv[1][len-1] == '/'){
      sprintf(votespath, "%svotes.txt", argv[1]);
    }
    else {
      sprintf(votespath, "%s/votes.txt", argv[1]);
    }
    printf("execing:\n./Leaf_Counter %s\n", votespath);
    execl("./Leaf_Counter", "Leaf_Counter", votespath, (char*)NULL);
    perror("Exec failed.\n");
  }
  else {
    aggregateVotes(argv[1]);
  }

  return 0;
}
