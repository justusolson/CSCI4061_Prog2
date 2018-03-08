#define NUM_ARGS 1
#define MAX_CANDIDATES 100
#define _GNU_SOURCE

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
Function: votesFileCheck
Description: function checks if a "votes.txt" file exists at the path
            passed to it.
returns 1 if it does exist
returns 0 if it does not exist
************************************************************************/
int votesFileCheck(char* path){
  DIR* direntStream = opendir(path);
  struct dirent* thisDir;
	while(1)
	{
		thisDir = readdir(direntStream);
		if(thisDir==NULL){ //end of directory
			break;
		}
    else if(thisDir->d_type == DT_REG){//if a file is a regular flie
      if(strcmp(thisDir->d_name, "votes.txt")==0){
        return 1; //found a file with the name votes.txt
      }
    }
  }
  return 0;
}

/************************************************************************
Function: votesReadWrite
Description: function reads input from path/votes.txt and counts votes
            per candidate. Outputs these votes to a text file whose name
            is the same as the last directory in the path.
            Ex: path: /Who_Won/Region_1/County_1
                output: /Who_Won/Region_1/County_1.txt

returns the character string of the output file path
************************************************************************/
char* votesReadWrite(char* path){
  DIR* direntStream = opendir(path);
	struct dirent* thisDir;
	FILE *input, *output;
  char candidateNames[MAX_CANDIDATES][1024];
  int candidateVotes[MAX_CANDIDATES];
  int i;
  for(i=0; i<MAX_CANDIDATES; i++){//initialize candidateVotes array to 0
    candidateVotes[i]=0;
  }
	while(1)
	{
		thisDir = readdir(direntStream);
		if(thisDir==NULL){
			break;
		}
		else if(thisDir->d_type == DT_REG && strcmp(thisDir->d_name, "votes.txt")==0){
			int length = strlen(path)+strlen(thisDir->d_name);
			char* inPath = malloc((length+10)*sizeof(char));	//allocates space for full path name
			sprintf(inPath, "%s/%s",path,thisDir->d_name);		//creates string for full path
			input = fopen(inPath, "r");

      char** args;
      int n = makeargv(path, "/", &args);
			length = strlen(path)+strlen(args[n-1])+strlen(".txt");

			char* outputPath = malloc((length+10)*sizeof(char));	//allocates space for full path name
			sprintf(outputPath, "%s/%s.txt",path,args[n-1]);		//creates string for full path
			output = fopen(outputPath, "w");
      free(*args);
      free(args);
      printf("output: %s\n", outputPath);
      if(output==NULL){
        perror("Error opening file\n");
        return;
      }
			char* temp = malloc(1024*sizeof(char));
			size_t length2;

			while(getline(&temp, &length2, input)!= -1){
        temp = trimwhitespace(temp);
        for(i=0; i<MAX_CANDIDATES; i++){
          if(candidateVotes[i]>0 && strcmp(candidateNames[i],temp)==0){//if they have the same name
            candidateVotes[i]++;//add a vote to that candidate
            break;
          }
          else if(candidateVotes[i]==0){//if we find an empty index in array then we have a new candidate
            candidateVotes[i]++;
            strcpy(candidateNames[i],temp);
            break;
          }
        }
			}

      free(temp);
      char* outputString = malloc(MAX_CANDIDATES*1024*sizeof(char));
      i=0;
      while(candidateVotes[i]>0){
        if(i>0){
          sprintf(outputString, "%s,%s:%d",outputString, candidateNames[i], candidateVotes[i]);
        }
        else{
          sprintf(outputString, "%s%s:%d",outputString, candidateNames[i], candidateVotes[i]);
        }
        i++;
      }
      sprintf(outputString, "%s\n", outputString);
      fputs(outputString, output);
			fclose(output);
			fclose(input);
			free(inPath);
      free(outputString);
      return outputPath;
		}
	}
	return;

}

int main(int argc, char** argv){
  if(argc < NUM_ARGS +1){ //checks for proper number of arguments
    printf("Wrong number of args to leafcounter, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
  }
  int len = strlen(argv[1]);
  char* path = malloc(len*sizeof(char));
  strcpy(path, argv[1]);
  if(path[len-1] == '/'){
    path[len-1]='\0';
  }
  if(votesFileCheck(path)){//determines if votes.txt exists in the directory
    char* filename = votesReadWrite(path);
    printf("%s\n",filename);
    free(filename);
  }
  else{
    printf("Not a leaf node.\n");
  }
  free(path);
  return 0;
}
