#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>

#define NUM_ARGS 1
#define MAX_CANDIDADTES 100

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
  char* candidateNames[MAX_CANDIDADTES];
  int candidateVotes[MAX_CANDIDADTES];
  int i;
  for(i=0; i<MAX_CANDIDADTES; i++){//initialize candidateVotes array to 0
    candidateVotes[i]=0;
  }
  printf("In votesReadWrite\n");
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
			printf("inPath: %s\n",inPath);
			input = fopen(inPath, "r");

			length = strlen(path)+strlen(".txt");
			char* outputPath = malloc((length)*sizeof(char));	//allocates space for full path name
			sprintf(outputPath, "%s.txt",path);		//creates string for full path
			printf("outputPath: %s\n",outputPath);
			output = fopen(outputPath, "w");
			char* temp;
			size_t length2;

			while(getline(&temp, &length2, input)!= -1)
			{
        printf("readline: %s\n",temp);
        for(i=0; i<MAX_CANDIDADTES; i++){
          if(candidateVotes[i]==0){//if we find an empty index in array then we have a new candidate
            candidateVotes[i]++;
            candidateNames[i]=temp;
            break;
          }
          else if(strcmp(candidateNames[i],temp)==0){//if they have the same name
            candidateVotes[i]++;//add a vote to that candidate
            break;
          }
        }
			}
      char* outputString = malloc(MAX_CANDIDADTES*1024*sizeof(char));
      i=0;
      while(candidateVotes[i]>0){
        if(i>0){
          sprintf(outputString, ",%s%s:%d",outputString, candidateNames[i], candidateVotes[i]);
        }
        else{
          sprintf(outputString, "%s%s:%d",outputString, candidateNames[i], candidateVotes[i]);
        }
        i++;
      }
      sprintf(outputString, "%s\n", outputString);
      printf("Testing: %s", outputString);
      fputs(outputString, output);
      char* returnString = outputString;
			fclose(output);
			fclose(input);
			free(inPath);
			free(outputPath);
      free(outputString);
      return returnString;
		}
	}
	return;

}

int main(int argc, char** argv){
  if(argc < NUM_ARGS +1){ //checks for proper number of arguments
    printf("Wrong number of args to leafcounter, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
  }

  if(votesFileCheck(argv[1])){//determines if votes.txt exists in the directory
    // printf("Found Votes.txt\n");
    char* filename=votesReadWrite(argv[1]);
  }
  else{
    printf("Not a leaf node.\n");
  }
  return 0;
}
