#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define TOTAL_CUSTOM_COMMANDS 6
#define CUSTOM_COMMAND_SIZE TOTAL_CUSTOM_COMMANDS*2 
#define COMMAND_MAX_SIZE 1000
#define MAX_ARGUMENT_SIZE 10 

/* Buffer and pointer array declared global to store command */
char command_buffer[COMMAND_MAX_SIZE];
char argument_buffer[COMMAND_MAX_SIZE];
char* argVec[MAX_ARGUMENT_SIZE];

/* Array of custom_commands in which we can add few other commands as well
 * To add any other custom command simply Change TOTAL_CUSTOM_COMMAND 
 * and add the command name along with it's bash equivalent command */
char* custom_command[CUSTOM_COMMAND_SIZE] = {
  "help","print_help",
  "diskuse","du -csh",
  "jobtree","ps -o user:32,pid,stime,tty,cmd -U $USER --forest",
  "drives","df -h -T ext4",
  "lastmod","find / -mtime -$DAYS -ls",
  "logout","ps -ef -u $USER"
};

/* Helper Method to clean up argVec array */
void clean_up_argVec()
{
	for(int i = 0 ; i < MAX_ARGUMENT_SIZE ; i++ )
	{
		if(argVec[i] != NULL )
		{
			memset(argVec[i],0,strlen(argVec[i]));
      argVec[i] = NULL ; 
		}
	}
}

/* Helper Debug Method to print argVec array*/
void print_up_argVec()
{
	for(int i = 0 ; i < MAX_ARGUMENT_SIZE ; i++ )
	{
		if(argVec[i] != NULL )
		{
			printf("argVec[%d] = %s \n", i , argVec[i]);
		}
	}
}

/* Helper Method to print help information */
void print_help()
{
  printf("Supporting commands are : \n");
  for(int i = 0, count = 1  ; i < CUSTOM_COMMAND_SIZE ; i+=2, count++ )
  {
    printf("| %d. %s\t\t| ", count , custom_command[i]);
    printf(" %s \n", custom_command[i+1]);
  }
};

/* Helper Method to split command and store the address of each argument in argVec
 * This method also updated $USER and -$DAYS with the default or user specified argument 
 * Further handling can be added here to support more custom arguments for custom commands */
void split_command_args(char* command)
{
	int i ;
	if(command == NULL )
	{
		return ; 
	}
	for (i = 0; i < MAX_ARGUMENT_SIZE; i++) 
	{
		argVec[i] = strsep(&command, " ");
    if( argVec[i] != NULL && strcmp(argVec[i] ,"$USER") == 0 )
    {
      memset(argVec[i],0,strlen(argVec[i]));
      if(strlen(argument_buffer) == 0   )
      {
        //Copy the current username
        strcpy(argument_buffer, getenv("USER")) ;
      }
      argVec[i]=argument_buffer;
      printf("Variable used for USER = %s\n", argVec[i]);
    }
    if(  argVec[i] != NULL && strcmp(argVec[i] ,"-$DAYS") == 0 )
    {
      memset(argVec[i],0,strlen(argVec[i]));
      if( strlen(argument_buffer) == 0  )
      {
        //Copy the current username
        strcpy(argument_buffer, "-1") ;
      }
      argVec[i]=argument_buffer;
      printf("Variable used for DAYS = %s\n", argVec[i]);
    }
		if (argVec[i] == NULL)
			break;
		if (strlen(argVec[i]) == 0)
			i--;
	}
}

/* Helper method to init shell and print help message on first time */
void custom_shell_init()
{
  printf("Starting custom Shell\n");
  print_help();
};

/* Helper method to take input from user and store the command in command_buffer 
 * Before input argVec and command_buffer is cleaned up for fresh input */
void getInput(char *command_buffer) 
{
  clean_up_argVec();
  //print_up_argVec();
	memset(command_buffer,0,COMMAND_MAX_SIZE);
  size_t length ; 
  printf(">> ");
  size_t len = getline(&command_buffer, &length , stdin ) ;
  if((len > 0) && (command_buffer[len-1] == '\n'))
  {
    command_buffer[len-1] ='\0';
  }
};

/* Helper Method to check if the user input command is present in the custom_command array 
 * This method also checks if any additional argument is sent with custom_command and stores
 * it in the argument_buffer. */
int check_custom_command(char** command)
{
  int command_code = -1; 
	if(command[0] == NULL )
		return -1 ; 
  for(int i=0; i < CUSTOM_COMMAND_SIZE; i+=2 )
  {
    int res = strcmp(command[0],custom_command[i]);
    if(res == 0 )
    {
      command_code = i ;
      if(command[1] != NULL )
      {
        memset(argument_buffer,0,COMMAND_MAX_SIZE);
        strcpy( argument_buffer,  command[1]) ;
      }
      break; 
    }
  }
  return command_code ; 
};

/* Helper Method to execute command using execvp command 
 * In this parent process waits for the forked child process to complete execution */
void execute_command(char** command)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1)
	{
		printf("\nFailed forking child..");
		return;
	}
	else if (pid == 0)
	{
    //Forked Child process 
		if (execvp(command[0],command ) < 0)
		{
			printf("\nCould not execute command..\n");
		}
		exit(0);
	}
	else
	{
    //Parent process 
		// waiting for child to terminate
		wait(NULL);
		return;
	}  
};

/* Helper Method to execute the bash equivalent command mapped to custom_command user provided 
 * In this method we clean up command_buffer and update it with bash equivalent custom_command */
void execute_direct_custom(int index,char** command)
{
	memset(command_buffer,0,COMMAND_MAX_SIZE);
	strcpy(command_buffer, custom_command[index+1]);
  split_command_args( command_buffer )  ;
  execute_command(argVec);
}

/* Helper Method to execute respective custom_command along with respect to the command code */
void execute_custom_command(int command_code, char** command )
{
	switch(command_code)
	{
		case 0: //help case
			print_help();
			break;
		case 2: //diskuse
			execute_direct_custom(command_code,command);
			break ;
		case 4: //jobtree
      execute_direct_custom(command_code,command);
			break;
		case 6: //drives
      execute_direct_custom(command_code,command);
			break;
		case 8: //lastmod
      execute_direct_custom(command_code,command);
			break ; 
		case 10: //exit_shell
      printf("Current User Processes running \n");
      execute_direct_custom(command_code,command);
			printf("###### Exiting Custom Shell ######\n");
			exit(0);
			break;
	}
};

/* Helper Method to process command user entered 
 * This method calls method to split the command 
 * Check wether it is a custom_command or system_command 
 * Executes the command accordingly */
void process_command(char* command )
{
  split_command_args(command);
  int command_code = check_custom_command(argVec); 
  if (command_code != -1  )
  {
    printf("Executing custom command : %s\n", command);
    execute_custom_command(command_code, argVec );
  }
  else
  {
    printf("Executing system command \n");
    execute_command(argVec);
  }
}

/* Main function for flow start */
int main(int argc , char* argv[] )
{
    custom_shell_init();
    while(1)
    {
      getInput(command_buffer);
      process_command(command_buffer);
    }
    return 0 ; 
}
