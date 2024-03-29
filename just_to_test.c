#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "manager.h"
#define MAX_SERVERS 100
#define MIN_REPLICAS 2
#define STR_BUFFER_SIZE 255 // A linux file cannot be >255 characters long
#define MAX_ARGS 5

/*****************************************************
* Main server manager that creates all servers
* Manages all structs associated with server instances
* Author: Gloire Rubambiza
* Version: 09/28/2017
******************************************************/

/*****************************************************
Fills the server's struct for housekeeping purposes
* Server manager uses this to keep track of the server
* @param server the struct to be filled
* @param parent_pid the server's manager aka its parent
* @param arguments the parameters passed by the user
******************************************************/
void fill_struct(Server* server, const char* name, int limits[]){
  server->name = malloc(sizeof(ServerName));
  strcpy(server->name, name);
  server->active_processes = limits[0];
  server->max_process = limits[1];
}

/**
* Updates all server structs with newly created pids
* @param server the struct to be updated
*/
void update_struct (Server* server, pid_t* pid){
  server->server_pid = *pid;
}

/**
* Sends the server to execute in a different process
@param server the server to be created
@return 0 on successful creation of server, otherwise error
*/
void creater_server (Server* server, char* tokens[]){

  if (execvp(tokens[0], tokens) < 0) {
        fprintf(stderr, "Error: Could not start server.\n");
        exit(-1);
  };
}

/**
 * Displays a prompt for the user to input commands
*/
void display_prompt(){
  printf("Please enter the next command --> ");
}

/**
 * Reads and parses the command entered by the user, and stores it in
 * pre-allocated buffers.
 * @param tokens is the pointer to the array that holds the user's tokenized
 *        command.
 * @return -1 if there's an error, 1 if no command was entered, 0 otherwise.
 */
int read_command(char* tokens[]) {
    int max_size = STR_BUFFER_SIZE * MAX_ARGS;
    char* command_buffer = malloc(max_size);
    if (fgets(command_buffer, max_size, stdin) == NULL) {
        fprintf(stderr, "There was an error reading user input.\n");
        return -1;
    }
    int parse_result = parse_command(command_buffer, tokens);
    if (parse_result < 0) {
        fprintf(stderr, "Could not parse the command.\n");
    }
    return parse_result;
}

/**
 * Parses the user input into an array of string tokens that are separated by
 * tab, newline or space characters.
 * @param command_buffer is the user input.
 * @param tokens is the array that will hold the parsed tokens.
 * @return -1 if an error occurs, 1 if no command was entered, 0 otherwise.
 */
int parse_command(char* command_buffer, char* tokens[]) {
    int count = 0;
    char* delimiter = "\t \n";
    char* current_token = strtok(command_buffer, delimiter);
    if (current_token == NULL) {
        //resets tokens array to empty strings
        int i;
        for (i = 0; i < MAX_ARGS; ++i) {
            tokens[i] = NULL;
        }
        return 1;
    }
    while(current_token != NULL) {
        tokens[count] = current_token;
        current_token = strtok(NULL, delimiter);
        count++;
        if (count >= MAX_ARGS) {
            fprintf(stderr, "Too many arguments given!\n");
            return -1;
        }
    }
    return 0;
}

/**
 * Searches for the server's pid before sending a kill signal.
 */
pid_t search_server ( const char* name, Server manager[] ){
  int k;
  pid_t ts_pid;
  for ( k = 0; k < MAX_SERVERS; ++k) {
    if ( manager[k].name != NULL) { // Avoids comparing to null pointers.
        if( strcmp(manager[k].name,name) == 0){
        ts_pid = manager[k].server_pid;
	free(manager[k].name);
        return ts_pid ;
      }
    }
  }
  return -1;
}
int main(int argc, char* argv[]){

  // Global variables for server and process limits
  Server manager[MAX_SERVERS];
  int server_count = 0;
  int proc_limits[2];
  pid_t pid;
  const char* name;
  
  // Arguments to be passed to child processes via vector pointer
  char* server_args[MAX_ARGS];
  int i;
  for ( i = 0; i<4; ++i){
    server_args[i] = malloc(sizeof(char));
  }
  
  server_args[0] = "./server.o";
  strcpy(server_args[1], argv[2]);
  strcpy(server_args[2], argv[3]);
  strcpy(server_args[3], argv[4]);
  


  printf("[Server Manager]: Started manager, creating servers soon\n" );

  if ( strcmp(argv[1],"createServer") == 0){

    // Check that the min_process passed in is not less than the default
    if ( (atoi(argv[3])) < MIN_REPLICAS) {
      printf("[Server Manager]: Minimum processes set to default of 2\n");
      proc_limits[0] = MIN_REPLICAS;
    } else {
      proc_limits[0] = atoi(argv[3]);
    }
    
    // Fill the globals before passing them to fill struct
    name = argv[2];
    proc_limits[1] = atoi(argv[4]);
    fill_struct(&manager[server_count], name, proc_limits);
  }
  
  if ( ( pid = fork()) < 0){
    fprintf(stderr, "Forking the server failed!\n");
  }

  if ( pid > 0 ) { // The parent keeps executing from here
      // Update the child's struct
      update_struct(&manager[server_count], &pid);
      server_count++;

     // Keep waiting for user input for the next command
     while (1) {
       char* tokens[MAX_ARGS];
       display_prompt();
       /*if (read_command(tokens) != 0) {
           continue;
       }*/
       const char temp_name[20], my_command[20];
       int minactive, maxactive;
       scanf("%s %s %d %d", temp_name, my_command, &minactive, &maxactive);
       printf("name is %s and max is %d\n", temp_name, maxactive);
       // Check for all possible operations
       /*if ( strcmp(usr_cmd, "createServer") == 0){
         // Assign arguments for the struct of the given server 
	 strcpy(server_args[1], name);
         sprintf(server_args[2], "%d", proc_limits[0]);
	 printf("My min is %s", server_args[2]);
	 sprintf(server_args[3], "%d", proc_limits[1]);
	 printf("My max is %s", server_args[3]);
       }*/
       if ( strcmp(tokens[0], "abortServer") == 0){
	 // Check that the min_process passed in is not less than the default.
         name = tokens[1];
 
	 // Search for server to send a kill signal.
	 pid_t target_server_pid = (search_server(name, manager)); 
	 if ( target_server_pid < 0){
	   fprintf(stderr, "ERROR: no server found under name %s\n", name);
	 } else { // Send the signal for the server to shut down.
           
	   kill(target_server_pid, SIGINT);

           // Decrement the number of servers in the pool.
           server_count--;
	 }
       } else if ( strcmp(tokens[0], "createProcess") == 0){
         // Do stuff for createProcess
       } else if ( strcmp(tokens[0], "abortProcess") == 0){
         // Do stuff for abort process
       } else if ( strcmp(tokens[0], "displayStatus") == 0 ){ 
         // Do stuff for display status
       }
	
     }
   }
  if ( pid == 0) { // The child starts the server in here
      creater_server(&manager[server_count], server_args);
  }
  sleep(15);
  return 0;



}
