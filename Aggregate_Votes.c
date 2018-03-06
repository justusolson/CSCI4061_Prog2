#define NUM_ARGS 1
#define MAX_CANDIDATES 100

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include "makeargv.h"

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
  int candidateVotes[MAX_CANDIDATES];
  int numberOfCandidates = 0;
  char* candidateNames[MAX_CANDIDATES][2];
  int i;
  for(i=0; i<MAX_CANDIDATES; i++){
    candidateVotes[i] = 0;
    // candidateNames[i] = malloc(1024);
  }
  char* subresultsfile, newresultsfile;

  DIR* curdir = opendir(path);
  struct dirent* subdir;

  while(1){
    subdir = readdir(curdir);
    if(subdir == NULL){
      break;
    }

    if(strcmp(subdir->d_name, ".")!=0 && strcmp(subdir->d_name, "..")!=0){
      if(subdir->d_type == DT_DIR){
        pid_t pid = fork();
        if(pid == 0){
          char* newpath = malloc(strlen(path)+strlen(subdir->d_name)+10);
          sprintf(newpath, "%s%s", path, subdir->d_name);
          if(isLeaf(newpath)){
            printf("newpath: %s\n", newpath);
            execl("./Leaf_Counter", "Leaf_Counter", newpath, (char*)NULL);
            perror("Exec Leaf_Counter failed.\n");
          }
          else {
            printf("newpath: %s\n", newpath);
            execl("./Aggregate_Votes", "Aggregate_Votes", newpath, (char*)NULL);
            perror("Exec Aggregate_Votes failed.\n");
          }
        }
        else {
          wait(NULL);
          sprintf(subresultsfile, "%s%s.txt", path, subdir->d_name);
          path[strlen(path)-1] = 0;
          sprintf(newresultsfile, "%s.txt", path);
          FILE* subresults = fopen(subresultsfile, "r");
          if(subresults == NULL){
            printf("error opening file %s\n", subresultsfile);
            exit(0);
          }
          char* line;
          size_t len = 0;
          while(getline(&line, &len, subresults)!= -1){
            trimwhitespace(line);
            char** candidateArray;
            int n = makeargv(line, ",", candidateArray);
            if(numberOfCandidates == 0)
              numberOfCandidates = n;
            int j,k;
            for(j=0; j<n; j++){
              char** temp;
              makeargv(candidateArray[k], ":", temp);
              printf("temp[0]: %s, temp[1]: %s", temp[0], temp[1]);
              for(k=0; k<MAX_CANDIDATES; k++){
                if(candidateVotes[k]>0 && strcmp(candidateNames[k],temp[0])==0){ //if they have the same name
                  candidateVotes[k] += atoi(temp[1]); //add a vote to that candidate
                  break;
                }
                else if(candidateVotes[k]==0){//if we find an empty index in array then we have a new candidate
                  candidateVotes[k] += atoi(temp[1]);
                  strcpy(candidateNames[k],temp[0]);
                  break;
                }
              }
            }
          }
          if(line)
            free(line);
        }
      }
    }
  }
  // print to output file
  FILE* newresults = fopen(newresultsfile, "w");
  if(newresults == NULL){
    printf("error opening file %s\n", newresultsfile);
    exit(0);
  }
  char* output;
  int l;
  for(l=0; l<numberOfCandidates-1; l++){
    sprintf(output, "%s:%d,", candidateNames[l], candidateVotes[l]);
  }
  sprintf(output, "%s:%d\n", candidateNames[numberOfCandidates-1], candidateVotes[numberOfCandidates-1]);
  fputs(output, newresults);
}

int main(int argc, char** argv)
{
  if(argc < NUM_ARGS+1){
    printf("Incorrect number of arguments, expected %d given %d.\n", NUM_ARGS, argc-1);
    return -1;
  }

  char* path = malloc(strlen(argv[1])+12);
  int len = strlen(argv[1]);
  // printf("last char of argv[1]: %c\n", argv[1][len-1]);
  if(argv[1][len-1] != '/'){
    sprintf(path, "%s/", argv[1]);
  }

  if(isLeaf(argv[1])){
    printf("isLeaf\n");
    char* votespath = malloc(strlen(argv[1])+12);
    sprintf(votespath, "%svotes.txt", path);
    printf("execing:\n./Leaf_Counter %s\n", votespath);
    execl("./Leaf_Counter", "Leaf_Counter", votespath, (char*)NULL);
    perror("Exec failed.\n");
  }
  else {
    aggregateVotes(path);

  }
  return 0;
}
