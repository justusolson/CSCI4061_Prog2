/* login: slett142, olso7127
 * Date: 02/21/18
 * Name: Travis Slettvedt, Justus Olson */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "makeargv.h"

#define MAX_NODES 100
#define INCOMPLETE 0
#define COMPLETE 1


//Function signatures

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Total Allocated Nodes
 * About parseInput: parseInput is supposed to
 * 1) Open the Input File [There is a utility function provided in utility handbook]
 * 2) Read it line by line : Ignore the empty lines [There is a utility function provided in utility handbook]
 * 3) Call parseInputLine(..) on each one of these lines
 ..After all lines are parsed(and the DAG created)
 4) Assign node->"prog" ie, the commands that each of the nodes has to execute
 For Leaf Nodes: ./leafcounter <arguments> is the command to be executed.
 Please refer to the utility handbook for more details.
 For Non-Leaf Nodes, that are not the root node(ie, the node which declares the winner):
 ./aggregate_votes <arguments> is the application to be executed. [Refer utility handbook]
 For the Node which declares the winner:
 This gets run only once, after all other nodes are done executing
 It uses: ./find_winner <arguments> [Refer utility handbook]
 */
int parseInput(char *filename, node_t *n);

/**Function : parseInputLine
 * Arguments: 's' - Line to be parsed
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Region Nodes allocated
 * About parseInputLine: parseInputLine is supposed to
 * 1) Split the Input file [Hint: Use makeargv(..)]
 * 2) Recognize the line containing information of
 * candidates(You can assume this will always be the first line containing data).
 * You may want to store the candidate's information
 * 3) Recognize the line containing "All Nodes"
 * (You can assume this will always be the second line containing data)
 * 4) All the other lines containing data, will show how to connect the nodes together
 * You can choose to do this by having a pointer to other nodes, or in a list etc-
 * */
int parseInputLine(char *s, node_t *n);

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes: parseInputLine is supposed to
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */
void execNodes(node_t *n, node_t* root_n);


int nameToID(char* s, node_t* n);
int determineRootNode(node_t* n);
int isLeaf(node_t* n, int id);
char* IO_tree(int id, node_t* n);
int cycle_recurse(int id, int* visited, int* anc, node_t* root_n);
int cycle(int* visited, int* anc, node_t* root_n);

//Variables

int currentlinefound = 0;
int numberOfCandidates = 0;
char* candidates;
int numberOfNodes = 0;
char* cwd;

//Functions

int parseInput(char* filename, node_t* n){
	FILE* input_file = file_open(filename);
	if(input_file == NULL){
		printf("error opening file\n");
		return 0;
	}
	char* line = malloc(sizeof(char)*1024);
	int len = 1, numNodes = 0, totalNumNodes = 0;
	while(len > 0){
		line = read_line(line, input_file); // get one line of input file
		// printf("line: %s\n", line);
		if(line == NULL)
			break;
		len = strlen(line);
		numNodes = parseInputLine(line, n); // numNodes is number of nodes allocated in parseInputLine
		totalNumNodes += numNodes; // add to total of number of nodes allocated
		// printf("linenum: %d, length: %d\n", i, len);
		// printf("line: %s, numNodes: %d\n", line, numNodes);
	}
	free(line); //deallocate memory
	if(totalNumNodes == 0){
		printf("Error with input file. Aborting.\n");
		return 0;
	}
	if(currentlinefound  < 3){
		printf("Error with input graph structure.\n");
		return 0;
	}
	int root_id = determineRootNode(n);
	// printf("root: %d\n", root_id);
	IO_tree(root_id, n); // assign input and output for each node

	int i;
	for(i=0; i<numberOfNodes; i++){ // for each node, assign its program with arguments
		char* p = malloc(1024); // could compute actual size
		if(n[i].id == root_id){ // root_id is Who_Won and needs the find_winner program
			sprintf(p, "./find_winner %d", n[i].num_children);
			int j;
			for(j=0; j<n[i].num_children; j++){
				sprintf(p, "%s %s", p, n[i].input[j]);
			}
			sprintf(p, "%s %s %d %s", p, n[i].output, numberOfCandidates, candidates);
			strcpy(n[i].prog, p);
		}
		else if(isLeaf(n, n[i].id) == 1){ // leaf nodes need the leafcounter program
			if(access(n[i].name, F_OK) != -1){
				strcpy(n[i].input[0], n[i].name);
				sprintf(p, "./leafcounter %s %s %d %s", n[i].input, n[i].output, numberOfCandidates, candidates);
				strcpy(n[i].prog, p);
			}
			else {
				printf("Missing input file for %s.\n", n[i].name);
				exit(0);
			}
		}
		else { // other nodes need the aggregate_votes program
			sprintf(p, "./aggregate_votes %d", n[i].num_children);
			int j;
			for(j=0; j<n[i].num_children; j++){
				sprintf(p, "%s %s", p, n[i].input[j]);
			}
			sprintf(p, "%s %s %d %s", p, n[i].output, numberOfCandidates, candidates);
			strcpy(n[i].prog, p);
		}
		free(p);//deallocate memory
	}
	return totalNumNodes; // total number of nodes allocated
}

char* IO_tree(int id, node_t* n){
	char* input;
	// printf("%s: \n", n[id].name);
	if(n[id].num_children == 0){
		// printf("\n");
		return n[id].output; // leaf nodes are the base case for recursion
	}
	else {
		int i;
		for(i=0; i<n[id].num_children; i++){ // node needs input file for each child
			int child_id = n[id].children[i];
			// printf("%s, ", n[child_id].name);
			strcpy(n[id].input[i], IO_tree(child_id, n)); // recursively call to get output file from children
		}
		// printf("\n");
		return n[id].output;
	}
}

int parseInputLine(char* s, node_t* n){
	trimwhitespace(s);
	char** strArray;
	makeargv(s, " ", &strArray);  // split line by spaces
	if(strArray[0][0] == '#' || strcmp(strArray[0], "\n") == 0){ // ignore commented and blank lines
		//printf("commented line\n");
	}
	else {
		if(currentlinefound == 0){ // first data line found is number and name of candidates
			numberOfCandidates = atoi(strArray[0]);
			candidates = malloc(strlen(s));
			if(strArray[1] == NULL){
				printf("Incorrect number of candidate names.\n");
				exit(0);
			}
			strcpy(candidates, trimwhitespace(strchr(s, ' ')));
			char** cand_test;
			makeargv(candidates, " ", &cand_test);
			char* temp = cand_test[0];
			int test = 0;
			while(temp != NULL){
				test++;
				temp = cand_test[test];
			}
			// printf("num cand: %d\n", test);
			if(test != numberOfCandidates || test == 0){
				printf("Incorrect number of candidates.\n");
				exit(0);
			}
			// printf("# candidates: %d, candidates: %s\n", numberOfCandidates, candidates);
			currentlinefound++;
			return 0;
		}
		else if(currentlinefound == 1) { // second data line found is names of all nodes
			int node_num = 0;
			while(strArray[node_num] != NULL){ // create new node and set characteristics
				node_t new_node;
				strcpy(new_node.name, strArray[node_num]);
				new_node.id = node_num;
				new_node.status = INCOMPLETE;
				new_node.num_children = 0;
				char* out = malloc(sizeof(char)*100);
				strcpy(out, new_node.name);
				prepend(out, "Output_");
				strcpy(new_node.output, out);
				free(out); //deallocate memory
				n[node_num] = new_node;
				node_num++;
			}
			numberOfNodes = node_num;
			// int j;
			// for(j = 0; j < node_num; j++){
			// 	printf("name: %s, id: %d, Output: %s\n", n[j].name, n[j].id, n[j].output);
			// }
			currentlinefound++;
			return 1; // return 1 for root node allocated, because it isn't counted otherwise
		}
		else { // data lines after first two dictate stucture of tree
			int parent_id = nameToID(strArray[0], n);
			// printf("parent_id: %d\n", parent_id);
			int child_num = 2, region_num = 0;
			while(strArray[child_num] != NULL){ // assign children to parent
				n[parent_id].children[n[parent_id].num_children] = nameToID(strArray[child_num], n);
				n[parent_id].num_children++;
				child_num++;
				region_num++;
			}
			// printf("line %d: %s\n", currentlinefound, s);
			currentlinefound++;
			return region_num; // return number of child nodes allocated
		}
	}
}

int nameToID(char* s, node_t* n){
	int id, i;
	for(i=0; i<numberOfNodes; i++){
		if(strcmp(s, n[i].name) == 0){
			return n[i].id; // return id of node with name s
		}
	}

	return -1;
}

int determineRootNode(node_t* n){
	int temp[numberOfNodes];
	int i, j, k;
	for(k=0; k<numberOfNodes; k++){ // initialize temporary array to 0's
		temp[k] = 0;
	}
	for(i=0; i<numberOfNodes; i++){ // check each node
		// printf("%s (%d): ", n[i].name, n[i].id);
		for(j=0; j<n[i].num_children; j++){ // check children of each i node
			int child_id = n[i].children[j];
			// printf("%s (%d), ", n[child_id].name, n[child_id].id);
			if(temp[child_id] == 0)
				temp[child_id] = 1; // if node is in a children array, it isn't the root
		}
		// printf("\n");
	}
	for(k=0; k<numberOfNodes; k++){
		if(temp[k] == 0)
			return k;	//k is id of root node
	}
	return -1;
}

int isLeaf(node_t* n, int id){
	if(n[id].num_children == 0){
		return 1;	//if node has no children it is a leaf
	}
	else return 0;
}

void execNodes(node_t* n, node_t* root_n){
	int id = n->id;
	// printf("execNodes Called for %s\n", n->name);
	// if(isLeaf(n, n->id)==0){	//if a node isn't a leaf
		int i, len = n->num_children;
		for(i=0; i<len; i++){	//loop through its children

			pid_t pid = fork();		//fork for every child node

			if(pid == 0){	//if we are in a child process
				int child_id = n->children[i];	//set child id
				id = child_id;
				// printf("%s's children: %d\n",n[child_id].name ,n[child_id].num_children);
				if(root_n[child_id].num_children > 0){	//if this child has its own children, call execNodes on this child
					execNodes(&root_n[child_id], root_n);
				}
				else{
					char* program = malloc(1024);
					char** args;
					strcpy(program, root_n[id].prog);
					makeargv(program, " ", &args);
					// printf("Node: %s program: %s\n", root_n[id].name, args[0]);
					free(program);//deallocate memory
					execvp(args[0], args);	//exec
					perror("ERROR: ");
				}
			}
			else if(pid <0){
				printf("Error Forking \n");
				exit(0);
			}
			else{
				//parent
				// waitpid(pid,0,0);

			}
		}
		if(isLeaf(root_n,n->id)){
			printf("%s is leaf node\n", n->name); // no leaf should reach this point
		}
		else{
			for(i=0; i<len; i++){//waits for all children
				wait(NULL);
			}
		// printf("Executing parent node %s\n",n->name);
		char* program = malloc(1024);
		char** args;
		strcpy(program, n->prog);
		makeargv(program, " ", &args);
		// printf("Node: %s program: %s\n",n->name, args[0]);
		free(program); //deallocate memory
		execvp(args[0], args);	//exec
		perror("ERROR: ");
	}
	return;
}

int cycle_recurse(int id, int* visited, int* anc, node_t* root_n){
	if(visited[id] == 0){
		visited[id] = 1;
		anc[id] = 1;

		int i, lenchild = root_n[id].num_children;
		for(i=0; i<lenchild; i++){
			int child_id = root_n[id].children[i];
			if(visited[child_id] == 0 && cycle_recurse(child_id, visited, anc, root_n) == 1){
				return 1;
			}
			else if(anc[child_id] == 1){
				return 1;
			}
		}
	}
	anc[id] = 0;
	return 0;
}


int cycle(int* visited, int* anc, node_t* root_n){
	int i;
	for(i=0; i<numberOfNodes; i++){
		visited[i] = 0;
		anc[i] = 0;
	}
	for(i=0; i<numberOfNodes; i++){
		if(cycle_recurse(i, visited, anc, root_n) == 1){
			return 1;
		}
	}
	return 0;
}

int main(int argc, char **argv){

	//Allocate space for MAX_NODES to node pointer
	struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	//call parseInput
	int num = parseInput(argv[1], mainnodes);
	if(num == 0){
		return -1;
	}
	// printf("candidates: %s\n", candidates);
	int j;
	for(j = 0; j < numberOfNodes; j++){
		// printf("name: %s, id: %d\n   prog: %s\n", mainnodes[j].name, mainnodes[j].id, mainnodes[j].prog);
	}
	// printf("printgraph:\n");
	// printgraph(mainnodes, num);

	/* Extra Credit
	int* visited = malloc(numberOfNodes);
	int* anc = malloc(numberOfNodes);
	if(cycle(visited, anc, mainnodes)==1){
		printf("Graph contains cycle.\n");
		return -1;
	}
	else {
		printf("Graph does not contain a cycle.\n");
	}
	free(visited);
	free(anc);
	*/

	//Call execNodes on the root node
	int root = determineRootNode(mainnodes);
	execNodes(&mainnodes[root], mainnodes);

	return 0;
}
