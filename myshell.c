/********************************************************************************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*********************************************************************************************/

#include <stdio.h>	  
#include <stdlib.h>	
#include <string.h>	
#include<ctype.h>
#include <sys/wait.h> // wait()
#include <unistd.h>	  // system calls
#include <sys/types.h>
#include <signal.h> // signal()
#include <fcntl.h>	// close(), open()


//Templates and MACROS
#define MAXCOMM 4
#define fori(a,b) for(int i=a;i<b;i++)
#define forie(a,b) for(int i=a;i<=b;i++)
int STATUSCODE = 1;
#define scp(a,b) strcmp(a,b)
#define evp(a,b) execvp(a,b)
#define len(a) strlen(a)
#define MAXSTR 1024



//********************************************************************************************************************
/*
void getInput(char* Inp)
{
    char** command = malloc(8 * sizeof(char *));
    char* separator = " ";
    char* parsed;
    int index = 0;

    // Splits input according to given delimiters.
    parsed = strtok(Inp, separator);
    while (parsed != NULL)
    {
        command[index] = parsed;
        index++;

        parsed = strtok(NULL, separator);
    }

    command[index] = NULL;   
}

void TrimWhiteSpace(char* s)
{
    char *end;
    
    // Trim leading space
    while (isspace((unsigned char)*s))
    {
        s++;
    }

    if (*s == 0) // All spaces
        return s;

    // Trim trailing space
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

}
*/
void printDir()        //function to print directory
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("%s$", cwd);
}

int cd(char* path)
{
    return chdir(path);
}

void handle_sigint(int signo)
{
    // this function is for handling signals in child process
	exit(0);
}

//parser functions
void parseSpace(char* inputStr, char** cmdArray)
{
    int i = 0;
    int flag = 0;
    while (i < MAXSTR && flag == 0)
    {
        cmdArray[i] = strsep(&inputStr, " "); // Tokenizing inputStr and storing commands in cmdArray
        
        if (!cmdArray[i])
        {
            flag = 1;
            break;
        }
        
        if (len(cmdArray[i]) == 0)
        {
            i--;
        }
        
        i++;
    }
}



//*********************************************************************************************************************
int parseParallel(char* inputStr, char** cmdArray)
{
    int flag = 0;
    fori(0, MAXCOMM)
    {
        cmdArray[i] = strsep(&inputStr, "&&"); // Separate commands using '&&' for parallel execution
        if (!cmdArray[i])                      // Check for null condition; exit the loop if true
        {
            flag = 1;
            break;
        }
        
        if (len(cmdArray[i]) == 0)
        {
            i--;
        }
    }
    
    if (!cmdArray[1])
    {
        return 0;
    }
    return 1;
}


//*******************************************************************************************************************
int parseSequential(char* inputStr, char** cmdArray)
{
	int flag=0;
	fori(0,MAXCOMM)
	{
		cmdArray[i] = strsep(&inputStr, "##"); //separating commands through ## for sequential running of commands
		if (!cmdArray[i]){    //check for the null condition and if yes then we come out of while loop
			flag=1;
			break;
        }	
		if (len(cmdArray[i])==0){
			i--;
        }	
	}
	if (!cmdArray[1]){
		return 0;
    }
	return 1;
}
//*********************************************************************************************************************
int parseRedirect(char* inputStr, char** cmdArray)
{
	
	int flag=0; 
	int i=0;
	while(i<2 && flag==0)
	{
		cmdArray[i] = strsep(&inputStr, ">"); //using > to redirect the output to a user specified file
		if (!cmdArray[i])
        {
			break;
        }
		if (len(cmdArray[i])== 0)
        {
			i--;
        }	
		i++;	
	}
	if (!cmdArray[1])
    {
		return 0;
    }
	return 1;
}
//************************************************************************************************************************

// This function will parse the input string into multiple commands or a single command with arguments based on && ## etc.

int parseInput(char* str, char** commands[])
{
	char* parser[MAXCOMM];

	int parallel_Checker=0;
	parallel_Checker = parseParallel(str, parser); //parallel checking
	if (parallel_Checker==1)
	{
		// for handling if the string is a parallel chain of commands
		int numCommands = 0;
		int flag=0;
		while (numCommands < MAXCOMM && flag==0)
		{
			if (!parser[numCommands])
            {
				flag=1;
				break;
            }
			parseSpace(parser[numCommands], commands[numCommands]);
			numCommands++;
		}
		return 0;
	}
    int sequential_Checker=0;
	sequential_Checker = parseSequential(str, parser); //sequential checking
	if (sequential_Checker==1)
	{
		// for handling if the string is a sequential chain of commands
		int numCommands = 0;
		int flag=0;
		while (numCommands < MAXCOMM && flag==0)
		{
			if (!parser[numCommands])
            {
				flag=1;
				break;
            }
			parseSpace(parser[numCommands], commands[numCommands]);
			numCommands++;
		}
		return 1;
	}
    
    int redirect_Checker=0;
	redirect_Checker = parseRedirect(str, parser);
	if (redirect_Checker==1)
	{
		// for handling the redirect case
		parseSpace(parser[0], commands[0]); 
		parseSpace(parser[1], commands[1]);
		return 2;
		
	}

	parseSpace(str, commands[0]); //for single command cases

	if (scp("exit",commands[0][0])==0) //for exit case
	{	
        return 3;
    }
	else if (scp("cd",commands[0][0] )==0) //for cd case
	{
		return 4;
    }	
	else
	{
		return 5; //for rest of the commands
	}
}

//**********************************************************************************************

//EXECUTABLE COMMANDS

void executeCommand(char** parsed)
{

	
	if (scp("exit", parsed[0]) == 0)       //exit command 
	{
		
		printf("Exiting shell...\n");
		STATUSCODE = -1;
		exit(0);
		
	}
	else if (scp(parsed[0], "cd") == 0)  // cd command
	{
	
		int status_code = chdir(parsed[1]);  //error(exception) handling
		if (status_code == -1)               //if status_code has value as -1 then the command is incorrect
		{
			printf("Shell: Incorrect command\n");
			exit(0);
		}
		
	}
	else
	{
		pid_t pid = fork(); //forking a child process
		
		if(pid==0){
		   	if (evp(parsed[0], parsed) < 0)
			{
				printf("Shell: Incorrect command\n"); //wrong command
			}
			exit(0); 
		}

		else if (pid < 0)
		{
			printf("Shell: Incorrect command\n"); //fork unsuccessful
		}

		else if(pid>0)
		{
			wait(NULL);
			return;
		}
	}
}

//**************************************************************************************************

// Function for running multiple commands in parallel
void executeParallelCommands(char** commandList[]) {
    int num = 0;
    int flag = 0;
    
    // Count the number of commands in the list
    while (num < MAXCOMM && flag == 0) {
        if (!commandList[num] || !commandList[num][0]) {
            flag = 1;
            break;
        }
        num++;
    }
    printf("Number of commands: %d\n", num);

    // Create a child process to execute the first command
    pid_t parentPID = fork();
    if (parentPID < 0) 
    {
        printf("Shell: Incorrect command\n");
    } 
    else if (parentPID == 0 && num >= 1) {
        // Child process: execute the first command
        if (scp("exit", commandList[0][0]) == 0) 
        {
            printf("Exiting shell...\n");
            STATUSCODE = -1;
            return;
        }
        else if (scp(commandList[0][0], "cd") == 0) 
        {
            // Handle 'cd' command
            int errorCode = chdir(commandList[0][1]);
            if (errorCode == -1) 
            {
                printf("Shell: Incorrect command\n");
            }
        } 
        else if (evp(commandList[0][0], commandList[0]) < 0) 
        {
            // Execute the command
            printf("Shell: Incorrect command\n");
        }
    } 
    else
    {
        if (scp("exit", commandList[1][0]) == 0) 
        {
            // Handle 'exit' command in the parent process
            printf("Exiting shell...\n");
            STATUSCODE = -1;
            return;
        } 
        else if (scp(commandList[1][0], "cd") == 0) 
        {
            // Handle 'cd' command in the parent process
            int errorCode = chdir(commandList[1][1]);
            if (errorCode == -1)
            {
                printf("Shell: Incorrect command\n");
            }
        } 
        else 
        {
            // Create a second child process to execute the second command
            pid_t childPID = fork();
            if (childPID == 0) 
            {
                // Child process: execute the second command
                if (evp(commandList[1][0], commandList[1]) < 0) 
                {
                    printf("Shell: Incorrect command\n");
                }
                return;
            } 
            else if (childPID && num > 2) 
            {
                if (strcmp("exit", commandList[2][0]) == 0) 
                {
                    // Handle 'exit' command in the parent process
                    printf("Exiting shell...\n");
                    STATUSCODE = -1;
                    return;
                } 
                else if (strcmp(commandList[2][0], "cd") == 0)
                {
                    // Handle 'cd' command in the parent process
                    int errorCode = chdir(commandList[2][1]);
                    if (errorCode == -1) 
                    {
                        printf("Shell: Incorrect command\n");
                    }
                } 
                else 
                {
                    // Create a third child process to execute the third command
                    pid_t child2PID = fork();
                    if (child2PID == 0) 
                    {
                        // Child process: execute the third command
                        if (evp(commandList[2][0], commandList[2]) < 0) 
                        {
                            printf("Shell: Incorrect command\n");
                        }
                        return;
                    } 
                    else if (child2PID && num > 3) 
                    {
                        if (scp("exit", commandList[3][0]) == 0) 
                        {
                            // Handle 'exit' command in the parent process
                            printf("Exiting shell...\n");
                            STATUSCODE = -1;
                            return;
                        } 
                        else if (scp(commandList[3][0], "cd") == 0) 
                        {
                            // Handle 'cd' command in the parent process
                            int errorCode = chdir(commandList[3][1]);
                            if (errorCode == -1) 
                            {
                                printf("Shell: Incorrect command\n");
                            }
                        } 
                        else 
                        {
                            // Create a fourth child process to execute the fourth command
                            pid_t child3PID = fork();
                            if (child3PID == 0)
                            {
                                // Child process: execute the fourth command
                                if (evp(commandList[3][0], commandList[3]) < 0) 
                                {
                                    printf("Shell: Incorrect command\n");
                                }
                                return;
                            } 
                            else if (child3PID != 0) 
                            {
                                // Wait for the fourth child process to finish
                                wait(NULL);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}



//*************************************************************************************************************
void executeSequentialCommands(char** cmd_args[])  // helps in running multiple commands
{
	
	int flag=0;
	int size = MAXCOMM;
	fori(0,size)
	{
		if (!cmd_args[i] || !cmd_args[i][0]){
			flag=1;
			break;
        }
		executeCommand(cmd_args[i]);   //if executeCommand() function gives value of statuscode as -1 then come out of loop
		if (STATUSCODE == -1){
			flag=1;
			break;
        }
	}
}

//**************************************************************************************************************
void executeCommandRedirection(char** cmd_args[])   //output redirected to user specified output file
{
	if( len(cmd_args[1][0])==0 || !cmd_args[1]  || !cmd_args[1][0] ) //if any of the three conditions are true then it is an incorrect command
	{
		printf("Shell: Incorrect command\n");
	}
	else
	{
		pid_t pid = fork();   //forking a child process
		if(pid!=0)
		{
		    wait(NULL);
		    return;
		}
		else if (pid == 0)
		{
			close(STDOUT_FILENO);                                //open and close can be used with help of #include <fcntl.h>
			open(cmd_args[1][0], O_CREAT | O_RDWR, S_IRWXU);

			if (evp(cmd_args[0][0], cmd_args[0]) < 0)   //evp is defined as execvp as macro
			{
				printf("Shell: Incorrect command\n");
			}
			return;
		}
	}
}
//*************************************************************************************************************
int main()
{
	// Initialization
	char** command_args[MAXCOMM + 1];
	char* buffer = NULL;
	char cwd[10000];        //current working directory cwd
	size_t buffer_size = 0;

	int i=0;
	while(i<MAXCOMM)
	{
		command_args[i] = (char**)malloc(MAXSTR * sizeof(char**));    //dynamically allocating space for command_args array
		i++;
	}
	command_args[MAXCOMM] = NULL;
	int flag=1;
	while (flag==1) 
	{
		
		getcwd(cwd, sizeof(cwd));
		printf("%s$", cwd);       //prints the present working directory on the terminal

		getline(&buffer, &buffer_size, stdin); 
         		
		buffer = strsep(&buffer, "\n");   //string separater function strsep separating string by && or ## or > etc.
		
		if (len(buffer) == 0)
		{
			continue;
		}

		int parseStatus = parseInput(buffer, command_args);   //invoking parseInput function it parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).

		if (parseStatus == 0)
		{
			executeParallelCommands(command_args); // This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
			if (STATUSCODE == -1) //status code becomes -1 if above function fails
			{
				break;
			}
		}
		else if (parseStatus == 1)
		{
			executeSequentialCommands(command_args); // This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
			if (STATUSCODE == -1)  //status code becomes -1 if above function fails
			{
				break;
			}
		}
		else if (parseStatus == 2)
		{
			executeCommandRedirection(command_args); //redirect the output to output file as specified by the user
			if (STATUSCODE == -1) //status code becomes -1 if above function fails
			{
				break;
			}
		}
		else if (parseStatus == 3) //comes to this case when user gives exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		else if (parseStatus == 4) //comes to this case when user gives cd command.
		{
		    int error_code=0;
			error_code = chdir(command_args[0][1]);
			if (error_code == -1)                          // if error code is -1 then command is incorrect
				printf("Shell: Incorrect command\n");
		}
		else
		{
			executeCommand(command_args[0]); // This function is invoked when user wants to run a single commands
		}
	}

	return 0;
}
