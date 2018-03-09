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
  if(curdir == NULL){
    printf("Failed to open path.\n");
    exit(0);
  }
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
  free(curdir);
  if(leaf) return 1;
  else return 0;
}

/************************************************************************
Function: aggregateVotes
Description: function recurses through directories forking and execing
  Leaf_Counter on leaves and Aggregate_Votes on non-leaves. Ignores
  votes.txt files in non-leaves. Child processes print to a txt file
  rather than writing to stdout.
input: Path to directory
output: Aggregation results in "<path>.txt" files and writes the output
  file path to stdout
************************************************************************/
void aggregateVotes(char* path){
  int candidateVotes[MAX_CANDIDATES];
  int numberOfCandidates = 0;
  char* candidateNames[MAX_CANDIDATES];
  int i;
  for(i=0; i<MAX_CANDIDATES; i++){
    candidateVotes[i] = 0;
    candidateNames[i] = malloc(1024);
  }
  char* subresultsfile = malloc(1024); //subdirectory <path>.txt file to read from
  char* newresultsfile = malloc(1024); //new <path>.txt file to write to

  DIR* curdir = opendir(path); //parent dir that aggregateVotes is called on
  if(curdir == NULL){
    printf("Failed to open path\n");
    exit(0);
  }
  struct dirent* subdir;  //each subdirectory or txt file

  while(1){
    subdir = readdir(curdir);
    if(subdir == NULL){ //reached the end of the directory
      break;
    }

    if(strcmp(subdir->d_name, ".")!=0 && strcmp(subdir->d_name, "..")!=0){ //ignore current and parent directories
      if(subdir->d_type == DT_DIR){ // for each subdirectory we exec either Aggregate_Votes or Leaf_Counter
        pid_t pid = fork();
        if(pid == 0){ // child process
          // char* newpath = malloc(strlen(path)+strlen(subdir->d_name)+10);
          sprintf(path, "%s/%s", path, subdir->d_name); //append subdirectory name to the path
          int fd = open("trash.txt", O_CREAT|O_WRONLY); //new txt file for child process to print to
          fchmod(fd, 0700);
          lseek(fd, 0, SEEK_END);
          dup2(fd, STDOUT_FILENO);
          if(isLeaf(path)){
            // printf("new leaf path: %s\n", path);
            execl("./Leaf_Counter", "Leaf_Counter", path, (char*)NULL);
            perror("Exec Leaf_Counter failed.\n");
          }
          else {
            // printf("new path: %s\n", path);
            execl("./Aggregate_Votes", "Aggregate_Votes", path, (char*)NULL);
            perror("Exec Aggregate_Votes failed.\n");
          }
        }
        else if(pid>0) {
          waitpid(pid,0,0);
          sprintf(subresultsfile, "%s/%s/%s.txt", path, subdir->d_name, subdir->d_name);
          // printf("subresultsfile: %s\n", subresultsfile);
          FILE* subresults = fopen(subresultsfile, "r");
          if(subresults == NULL){
            printf("error opening file %s\n", subresultsfile);
            exit(0);
          }
          free(subresultsfile);
          char* line = malloc(1024);
          size_t len = 0;
          getline(&line, &len, subresults);
          // printf("line: %s\n", line);
          trimwhitespace(line);
          char** candidateArray;
          // int p;
          // for(p=0; p<MAX_CANDIDATES; p++){
          //   candidateArray[p] = (char*)malloc(1024);
          // }
          int n = makeargv(line, ",", &candidateArray);
          free(line);
          fclose(subresults);
          if(numberOfCandidates == 0){
            numberOfCandidates = n;
            // printf("numCand: %d\n", n);
          }
          int j,k;
          for(j=0; j<n; j++){
            char** temp;
            // temp[0] = (char*)malloc(1024);
            // temp[1] = (char*)malloc(128);
            makeargv(candidateArray[j], ":", &temp);
            // printf("temp[0]: %s, temp[1]: %s\n", temp[0], temp[1]);
            for(k=0; k<MAX_CANDIDATES; k++){
              if(candidateVotes[k]>0 && strcmp(candidateNames[k],temp[0])==0){ //if they have the same name
                candidateVotes[k] += atoi(temp[1]); //add votes to that candidate
                break;
              }
              else if(candidateVotes[k]==0){//if we find an empty index in array then we have a new candidate
                // printf("candidate: %s, newvotes: %s, int votes: %d\n", temp[0], temp[1], atoi(temp[1]));
                candidateVotes[k] += atoi(temp[1]);
                strcpy(candidateNames[k],temp[0]);
                break;
              }
            }
            free(*temp);
            free(temp);
          }
          free(*candidateArray);
          free(candidateArray);
        }
        else{
          perror("Fork Failed\n");
          exit(0);
        }
      }
    }
  }
  char** args;
  int q = makeargv(path, "/", &args);
  sprintf(newresultsfile, "%s/%s.txt", path, args[q-1]);
  free(*args);
  free(args);
  FILE* newresults = fopen(newresultsfile, "w");
  if(newresults == NULL){
    printf("error opening file %s\n", newresultsfile);
    exit(0);
  }
  char* output = malloc(MAX_CANDIDATES*(1024+10)*sizeof(char));
  int l=0;
  while(candidateVotes[l]>0){
    if(l>0){
      sprintf(output, "%s,%s:%d",output, candidateNames[l], candidateVotes[l]);
    }
    else{
      sprintf(output, "%s:%d", candidateNames[l], candidateVotes[l]);
    }
    l++;
  }
  sprintf(output, "%s\n", output);
  int m;
  for(m=0; m<numberOfCandidates; m++){
    free(candidateNames[m]);
  }
  // printf("output string: %s\n", output);
  fputs(output, newresults);
  printf("%s\n", newresultsfile);
  free(newresultsfile);
  free(output);
  fclose(newresults);
}

int main(int argc, char** argv)
{
  if(argc < NUM_ARGS+1){
    printf("Incorrect number of arguments, expected %d given %d.\n", NUM_ARGS, argc-1);
    return -1;
  }

  int len = strlen(argv[1]);
  if(argv[1][len-1] == '/'){
    argv[1][len-1] = 0;
  }
  if(isLeaf(argv[1])){
    // printf("execing:\n./Leaf_Counter %s\n", path);
    execl("./Leaf_Counter", "Leaf_Counter", argv[1], (char*)NULL);
    perror("Exec failed.\n");
  }
  else {
    aggregateVotes(argv[1]);
    remove("trash.txt");
  }
  return 0;
}
