#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "commands.h"
#include "built_in.h"

static struct built_in_command built_in_commands[] = {
	{ "cd", do_cd, validate_cd_argv },
	{ "pwd", do_pwd, validate_pwd_argv },
	{ "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
	static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

	for (int i = 0; i < n_built_in_commands; ++i) {
		if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
			return i;
		}
	}

	return -1; // Not found
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{	
	char path_resol[5][512] = { "/usr/local/bin/", "/usr/bin/", "/bin/", "/usr/sbin/", "/sbin/"  };
	if (n_commands > 0) {
		struct single_command* com = (*commands);

		//If parsing result is 0, it terminates the program.
		assert(com->argc != 0);
		for( int i = 0; i < 5; i++ ) { 
		strcat(path_resol[i],com->argv[0]);
		}
		int built_in_pos = is_built_in_command(com->argv[0]);
		if (built_in_pos != -1) {
			if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
				if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
					fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
				}
			} else {
				fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
				return -1;
			}
		} else if (strcmp(com->argv[0], "") == 0) {
			return 0;
		} else if (strcmp(com->argv[0], "exit") == 0) {
			return 1;
		}
			//This part takes charge of the creation of new process.
		else if ( ( access( com->argv[0] , F_OK ) == 0 ) ) {
				int status;
				int pid;
				
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(com->argv[0],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
				return 0;
		}
		else if ( access( path_resol[0] , F_OK ) == 0 ) {
				int status;
				int pid;
				
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(path_resol[0],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
				return 0;
		}
		else if ( access( path_resol[1], F_OK ) == 0  ) {
				int status;
				int pid;
				
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(path_resol[1],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
				return 0;
		}
		else if ( access( path_resol[2], F_OK ) == 0  ) {
				int status;
				int pid;
				
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(path_resol[2],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
				return 0;
		}
		else if ( access( path_resol[3], F_OK ) == 0  ) {
				int status;
				int pid;
				
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(path_resol[3],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
				return 0;
		}
		else if ( access( path_resol[4], F_OK ) == 0  ) {
				int status;
				int pid;
				
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(path_resol[4],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
				return 0;
		}
		else {
				fprintf(stderr, "%s: command not found\n", com->argv[0]);
				return -1;
				}
				}
				
				return 0;
				}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
	for (int i = 0; i < n_commands; ++i) {
		struct single_command *com = (*commands) + i;
		int argc = com->argc;
		char** argv = com->argv;

		for (int j = 0; j < argc; ++j) {
			free(argv[j]);
		}

		free(argv);
	}

	memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
