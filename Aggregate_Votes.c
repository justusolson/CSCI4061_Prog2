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
  int leaf = NULL;
  while(1){
    thisdir = readdir(curdir);
    if(thisdir == NULL){
      if(leaf == NULL)
        leaf = 1;
      break;
    }
    if(strcmp(rd->d_name, ".")!=0 && strcmp(rd->d_name, "..")!=0){
			if(rd->d_type == DT_DIR){
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
    if(strcmp(rd->d_name, ".")!=0 && strcmp(rd->d_name, "..")!=0){
      else if(strcmp(rd->d_name, "votes.txt")==0){

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
    // execv();
  }
  aggregateVotes(argv[1]);

  return 0;
}
