/*
	Name: Michael Rao
	ID: 1001558150
*/
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"		//We want to split out cammand line up into tokens
								//So we need to define what delimits our tokens 
								//In this case white space 
								// will seperate the tokens on our command line 

#define MAX_COMMAND_SIZE 255    //The Max command-line size

#define MAX_NUM_ARGUMENTS 10	//Mav shell only supports five arguments 

#define MAX_NUM_HIST 15			//max number of history commands 

#define MAX_NUM_PIDS 15			//max number of pids stored 

int main()
{
	pid_t * pid_hist = (pid_t*)malloc(MAX_NUM_PIDS);

	char cmd_hist[MAX_NUM_HIST][MAX_COMMAND_SIZE];
	int hist_count = 0;
	int hist_index = 0;

	int pid_count = 0;
	int pid_cr_index = 0;

	int iter;
	for (iter = 0; iter < MAX_NUM_PIDS; iter++)
	{
		pid_hist[iter] = 0;
	}

	while(1)
	{
		char * cmd_str = (char*)malloc(MAX_COMMAND_SIZE);
		memset(cmd_str, 0, MAX_COMMAND_SIZE);
		//print out the msh prompt
		printf("msh> ");
		//read the command from the commandline 
		//The max command that will be read is MAX_COMMAND_SIZE
		//this while command will wait here until the user
		//inputs something since fgets returns NULL when there 
		//there is no input
		while(!fgets(cmd_str,MAX_COMMAND_SIZE,stdin));
		//********NOT WORKING***************** never enters loop 

		if (!strncmp(&cmd_str[0], "!",1))
		{
			int temp = atoi(&cmd_str[1]);
			strcpy(cmd_str, cmd_hist[temp]);
			printf("%d\n", temp);
		}
		
		/* parse input */
		char *token[MAX_NUM_ARGUMENTS];
		int token_count = 0;
		//pointer to point to the token 
		//parsed by strsep
		char *arg_ptr;

		char *working_str = strdup(cmd_str);
		memset(cmd_hist[hist_index], 0, MAX_COMMAND_SIZE);
		strncpy(cmd_hist[hist_index],working_str,strlen(working_str));  //recent change to strncpy
		hist_count++;
		hist_index++;
		if (hist_index >= MAX_NUM_HIST)
		{
			hist_index = 0;
		}
		free(cmd_str);
		//we are going to move the working_str pointer so
		//keep track of its original value so we can deallocate 
		//the correct amount at the end
		char *working_root = working_str;

		//tokenize the input string with whitespace used as the delimiter
		while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL)
			&& (token_count < MAX_NUM_ARGUMENTS))
		{
			token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
			if (strlen(token[token_count]) == 0)
			{
				token[token_count] = NULL;
			}
			token_count++;
		}
		if (token[0] == NULL)
		{
			continue;
		}
		//if exit or quit is input 
		//msh will break from while loop after freeing mem
		//inorder to exit with status 0
		if (!strcmp(token[0], "exit") || !strcmp(token[0], "quit"))
		{
			printf("exiting....\n");
			break;
		}
		//if cd is input msh will process this by using the chdir() function
		//due to exec family does not contain the functionality for cd
		//if there is an error or a proper path is not given 
		//chdir will return -1 subssequently triggering the error handling 
		else if (!strcmp(token[0], "cd"))
		{
			int dir_ret = chdir(token[1]);
			if (dir_ret == -1)
			{
				perror("cd failed: ");
			}
		}
		else if (!strcmp(token[0], "showpids"))
		{
			if (pid_hist[0] == 0 )
			{
				printf("No PID history\n");
			}
			else
			{
				int pid_index;
				if (pid_count >= MAX_NUM_PIDS)
				{
					for (pid_index = 0; pid_index < MAX_NUM_PIDS; pid_index++)
					{
						printf("%d. %d\n", pid_index, pid_hist[pid_count]);
					}
				}
				else
				{
					for (pid_index = 0; pid_index < pid_count; pid_index++)
					{
						printf("%d. %d\n", pid_index, pid_hist[pid_index]);
					}
				}
			}	
		}
		else if (!strcmp(token[0], "history"))
		{
			if (cmd_hist == NULL)
			{
				printf("No Command history\n");
			}
			else
			{
				int cmd_index;
				if (hist_count >= MAX_NUM_HIST)
				{
					for (cmd_index = 0; cmd_index < MAX_NUM_HIST; cmd_index++)
					{
						hist_index++;
						printf("%d. %s", cmd_index, cmd_hist[hist_index]);
						if (hist_index >= MAX_NUM_HIST)
						{
							hist_index = 0;
						}
					}
					//hist_index++;
				}
				else
				{
					for (cmd_index = 0; cmd_index < hist_index; cmd_index++)
					{
						printf("%d. %s", cmd_index, cmd_hist[cmd_index]);
					}
				}
			}

		}
		else
		{
			pid_t pid = fork();
			pid_hist[pid_cr_index] = pid;
			pid_count++;
			pid_cr_index++;
			if (pid_cr_index == MAX_NUM_PIDS)
			{
				pid_cr_index = 0;
			}

			if (pid == 0)
			{
				
				int ret = execvp(token[0], token);
				if (ret == -1)
				{
					printf("%s: Command not supported\n",token[0]);
				}
				return 0; //*** to prevent fork bombing ***
			}
			else
			{
				int status;
				wait(&status);
			}
		}
		free(working_root);
	}
	return 0;
}
