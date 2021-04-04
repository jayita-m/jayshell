//For main function: scroll to end 

//Libraries used
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//======================================================================================================================================================

char *read_cmdline(void)
{
  char *command = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer

  if (getline(&command, &bufsize, stdin) == -1){
    if (feof(stdin)) {  // Check for EOF
      exit(0);  //recieved an EOF
    } else  {
      perror("readline");
      exit(1);
    }
  }
  
  // Return the command as entered
  return command;
}

//======================================================================================================================================================

char **token_cmdline(char *line)
{
  int bufsize = 50, pos = 0;

  char **tokensBuf = malloc(bufsize * sizeof(char*)); //Allocate memory
  char *token;

  if (!tokensBuf) {  //Any pointer is null
    fprintf(stderr, "malloc error\n");
    exit(1);
  }

  // Parse the command line for each word
  token = strtok(line, " \n");
  while (token != NULL) {    // Till we receive NULL
    tokensBuf[pos] = token;  // Store token in buffer
    pos++;

    token = strtok(NULL, " \n");  // Read next token
  }
  tokensBuf[pos] = NULL;     // Put NULL in the last

  return tokensBuf;
}

//======================================================================================================================================================

int executeCmd(char **args)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
      // Child process to execute commands
      if (execvp(args[0], args) == -1) {
        perror("Invalid Command");
      }
      exit(1);
    } 

    else if (pid < 0) {
      // Error forking
      perror("Fork Error");
    } 

    else {
      // Parent process - wait for child process
      wait(NULL);
    }

    return 1;
}

//======================================================================================================================================================

int executePipeCmd(char *cmdline)
{
  char *cmdList[5];
  char **args;
  pid_t pid;

  char *token = strtok(cmdline, "|");
  int i=0;
  while(token != NULL) {
    cmdList[i] = token;
    token = strtok(NULL, "|");
    i++;
  }

  //for (int j=0; j<i; j++){
    //printf("cmdList[%d] = %s\n", j, cmdList[j]);
  //}

  if ( (pid = fork()) > 0 ) {
    //Parent process
    wait(NULL);
  }
  else if ( pid == 0 ) {
    int fd1[ 2 ];
    pipe( fd1 );

    //printf("Inside Child \n");

    if ( (pid = fork()) > 0 ) {
      if (dup2( fd1[ 0 ] , 0 ) < 0) {
        printf("Err dup2 in child\n");
      }
      close( fd1[ 0 ] );
      close( fd1[ 1 ] ); 

      args = token_cmdline(cmdList[1]);
      //printf("Second command: %s\n", cmdList[1]);
      
      if (execvp(args[0], args) == -1) {
        perror("Invalid Command");
      }

    }
    else if ( pid == 0 ) {
      //printf("Inside grand child\n");
      if (dup2( fd1[ 1 ] , 1 ) < 0) {
        
        printf("Err dup2 in gChild\n");
      }
      close( fd1[ 0 ] );
      close( fd1[ 1 ] );

      args = token_cmdline(cmdList[0]);
      if (execvp(args[0], args) == -1) {
        perror("Invalid Command");
      }
      exit(0);
    }
    exit(0);
  }
  return 1;
}

//======================================================================================================================================================

char* extractFilename(int type_num, char* cmdline){


  char* file_name = malloc(20*sizeof(char));
  //char cmdline[80];
  char* pChar;
  char* ptr;
  char* token;

  if(type_num == 1) //INPUT
    {
      pChar = "<";
      token = strtok(cmdline, pChar);
      token = strtok(NULL, "\n");
    }
  else if (type_num == 2) //OUTPUT
    {
      pChar = ">";
      token = strtok(cmdline, pChar);
      token = strtok(NULL, "\n");
    }
  else if (type_num == 3) //APPEND
    {
      pChar = ">>";
      ptr = strstr(cmdline, ">>");
      strtok(cmdline, ">");
      ptr = ptr+2;
      token = strtok(ptr, "\n");
    }
  

  //Trim any whitespaces by shifting pointer
  if (token[0] == ' ')
  {
    token++;
  }
  //printf("TOKEN IS === %s\n", token);

  //printf("TOKEN LENGTH IS === %ld\n", strlen(token));

  if (token == NULL)
  {
    return NULL;
  }

  else
  {
    //copy token in file_name by assigning pointer
    file_name = token;
  }
  //RETURNS THE FILE NAME FOLLOWING THE CHARACTER c 
  //If not found, return null 
  
  return file_name;
}

//======================================================================================================================================================

int redirectCmd(char* cmdline, int direct_type){

  //Find string after symbol
  //Check if symbol is output or input

  char *cmd;
  char **args;
  //int direct_type;

  //printf("Cmdline = %s  ", cmdline);
  char* filename = extractFilename(direct_type, cmdline);
  //printf("CmdlineAft = %s  ", cmdline);
  //printf("Returned file name ========%s\n", filename);


  args = token_cmdline(cmdline);
  //printf("FILE NAME IS = %s\n", filename);
  //printf("ARGS[0]=%s ARGS[1}=%s ARGS[2]=%s\n", args[0], args[1], args[2]);

  pid_t PID; 
  PID = fork();
  int fd;

  //CHILD PROCESS
  if(PID == 0)
  {
    if (direct_type == 3)   //IF REDIRECTION TYPE IS ==== OUTPUT
    {
      //OPEN FILE IN APPEND MODE 
      //printf("DIRECT TYPE IS = APPEND\n");
      if ((fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
        perror("Append: File open error");

      //LINK WITH STDOUT (1)
      dup2(fd, 1);

      //EXEC 
    }
    else if (direct_type == 2)   //IF REDIRECTION TYPE IS ==== OUTPUT
    {
      //OPEN FILE IN OUTPUT MODE 
      //printf("DIRECT TYPE IS = OUTPUT filename = %s\n", filename);
      fd = open(filename, O_WRONLY | O_CREAT, 0666);

      //LINK WITH STDOUT (1)
      dup2(fd, 1);

      //EXEC 
    }
    else if (direct_type == 1)    //IF REDIRECTION IS ===== INPUT
    {

      //printf("DIRECT TYPE IS = INPUT\n");
      
      if((fd = open(filename, O_RDONLY)) == -1)
      {
        perror("FILE DOES NOT EXIST");
        exit(0);
      }

      //LINK WITH STDIN (0)
      dup2(fd, 0);

      //EXEC 
    }

    
    if( (execvp(args[0], args)) == -1)
    {
      perror("Invalid command");
    }
    close(fd);
    exit(0);

  }

  else if(PID > 0){
    wait(NULL);

  }

  else {
      // Error forking
      perror("Fork Error");
    }

  //printf("COMPLETED CMD-REDIRECT FUNCTION\n");
  return 1;
}

//======================================================================================================================================================

int check(char *str, char ch)
{
	//Check if string contains a certain character
	int i, count_of_ch=0;
	for (i=0; str[i]; i++) 
	{
		if(str[i] == ch)
			count_of_ch++; //Increase count every time that character matches 
	}

	return count_of_ch;
}

//======================================================================================================================================================
int execute2PipeCmd(char* cmdline)
{
	char* cmdList[10];
	char** args0;
	char** args1;
	char** args2;
	pid_t pid;
	int status;

	char *token0 = strtok(cmdline, "|"); //Part before first pipe
	cmdList[0] = token0;

	char* token1 = strtok(NULL, "|"); //Part before second pipe
	cmdList[1] = token1;

	char* token2 = strtok(NULL, "\n"); //Part after second pipe
	cmdList[2] = token2;
	
	int pipes[4];
	pipe(pipes);
	pipe(pipes + 2);

	//EXECUTING FIRST COMMAND
	if (fork() == 0)
	{
		dup2(pipes[1], 1); 
		close(pipes[0]); close(pipes[1]); close(pipes[2]); close(pipes[3]);

		args0 = token_cmdline(cmdList[0]);
		if (execvp(args0[0], args0) == -1) {
			perror("Invalid Command");
		}
	}

	//PARENT
	else {
		//EXECUTING SECOND COMMAND
		if (fork() == 0)
		{
			dup2(pipes[0], 0); //Manipulating fd's
			dup2(pipes[3], 1);
			close(pipes[0]); close(pipes[1]); close(pipes[2]); close(pipes[3]);

			args1 = token_cmdline(cmdList[1]);
			if(execvp(args1[0], args1) == -1) {
				perror("Invalid command");
			}
		}

		else {
			if (fork() == 0)
			{
				dup2(pipes[2], 0);
				close(pipes[0]); close(pipes[1]); close(pipes[2]); close(pipes[3]);

				args2 = token_cmdline(cmdList[2]);
				if(execvp(args2[0], args2) == -1) {
					perror("invalid command");
				}
			}
		}
	} //BIG ELSE ENDS HERE

	close(pipes[0]); close(pipes[1]); close(pipes[2]); close(pipes[3]);
	for(int i = 0; i < 3; i++)
		wait(&status);

	return 1;
}

//======================================================================================================================================================
//======================================================================================================================================================
//======================================================================================================================================================


int main(int argc, char **argv)
{
	
	//List of variables user
  	char *cmdline;				//Stores input from command line
  	char **args;				//Stores separate arguments like "cat" or "ls"
  	char *cmdList[10];			//Temporary - used for store result of strtok()
  	int status=1;				//Checks whether program running smoothly
  	int first_time = 1;			//Checks if shell is just launched

  //Shell begins from here
  do {

  	//If first time then display colorful header (using ANSI sequences)
  	if(first_time == 1)
  	{
  		printf("\033[38;5;215;1m");
  		printf("\n");
  		printf("                                      *-----------------*                             ");
  		printf("\n");
  		printf("\n");
  		printf("     W E L C O M E      T O      J A Y S H     ----    J A Y I T A ' S      S H E L L\n");
  		printf("\n");
  		printf("                                      *-----------------*                             ");
  		printf("\n");
  		printf("\033[0m");
  		printf("\n");
  		first_time = 0;
  	}

  	//Display prompt
  	printf("\033[38;5;215;1m");
    printf("jayita-@-myShell#  ");
    printf("\033[0m");

    //Read input into cmdline
    cmdline = read_cmdline();

    //Check if pipe case
    if(strstr(cmdline, "|") != NULL){

    	//Checks if 1 or 2 pipes
    	int pipe_count = check(cmdline, '|');
    
    	if (pipe_count == 1) 
    	{
            //printf("Single Pipe Commands\n");
            status = executePipeCmd(cmdline);
            continue;
        }

        else if (pipe_count == 2)
        {
        	//printf("Double pipe command\n");
        	status = execute2PipeCmd(cmdline);
        	continue;
        }
    }


    //Checking if REDIRECTION case
    if (strstr(cmdline, ">>") != NULL) //APPEND - 3
    {
        status = redirectCmd(cmdline, 3);
        continue;
    }
    else if(strchr(cmdline, '>') != NULL) //OUTPUT - 2
    {
        status = redirectCmd(cmdline, 2);
        continue;
    }

    else if (strchr(cmdline, '<') != NULL) //INPUT - 1
    {
        status = redirectCmd(cmdline, 1);
        continue;
    }


    //IF USER KEEPS PRESSING "ENTER"
    if (strcmp(cmdline, "\n") == 0) continue;


    //If semicolon is present, tokenize there - else will return NULL and not tokenize
    char *cmd = strtok(cmdline, ";");
    int i=0;

    //Loop through command line
    while(cmd != NULL) {
      cmdList[i] = cmd;  		// Store each command
      cmd = strtok(NULL, ";");
      i++;
    }

    //SEMICOLON + NORMAL CASE
    for(int j=0; j<i; j++) {
      args = token_cmdline(cmdList[j]);  // Tokenize each command
      
      if (strcmp(args[0], "exit") == 0)  // Come out if command is exit
        exit(0);

      status = executeCmd(args);         // Execute the command
    }

    //Freeing pointers
    free(cmdline);
    free(args);

  } while (status); //Run while status = 1 i.e. not exit

  return 0;
}
