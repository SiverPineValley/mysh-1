#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

#include "commands.h"
#include "built_in.h"

#define SOCK_PATH "/tmp/test_server.dat"
#define BUFF_SIZE 1024

static struct built_in_command built_in_commands[] = {
	{ "cd", do_cd, validate_cd_argv },
	{ "pwd", do_pwd, validate_pwd_argv },
	{ "fg", do_fg, validate_fg_argv }
};
//PATH environment varialbe
static char path_resol[5][512] = { "/usr/local/bin/", "/usr/bin/", "/bin/", "/usr/sbin/", "/sbin/"  };

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
	int pid, status, num = 1, back = 0;
	char short_path[5][512] = { "" };
	struct stat buf[6];
	struct single_command* com = (*commands);
	back = isBack(com, n_commands);
	for( int i = 0; i < 5; i++ ) strcpy(short_path[i], path_resol[i]);
	if (n_commands == 1) {
		//If parsing result is 0, it terminates the program.
		assert(com->argc != 0);
		stat(com->argv[0], &buf[0]);
		//casting the resolution path
		for( int i = 0; i < 5; i++ ) {
		strcat(short_path[i],com->argv[0]);
		stat(short_path[i],&buf[i+1]);
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
			//This part takes charge of the creation of one new process.
		else if ( (( !S_ISDIR(buf[0].st_mode) ) && access( com->argv[0] , R_OK ) == 0 ) ) {			
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(com->argv[0],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
		}
		else if ( ( !S_ISDIR(buf[1].st_mode))&&access( short_path[0] , R_OK ) == 0 ) {
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(short_path[0],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
		}
		else if ( ( !S_ISDIR(buf[2].st_mode))&&access( short_path[1] , R_OK ) == 0 ) {
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(short_path[1],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
		}
		else if ( ( !S_ISDIR(buf[3].st_mode))&&access( short_path[2] , R_OK ) == 0 ) {
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(short_path[2],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
		}
		else if ( ( !S_ISDIR(buf[4].st_mode))&&access( short_path[3] , R_OK ) == 0 ) {
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(short_path[3],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
		}
		else if ( ( !S_ISDIR(buf[5].st_mode))&&access( short_path[4] , R_OK ) == 0 ) {
				pid = fork();
				if ( pid > 0 ) wait(&status);
				else if ( pid == 0 ) execv(short_path[4],com->argv);
				else {
				perror("Fork error");
				return -1;
				}	
		}
		else {
				fprintf(stderr, "%s: command not found\n", com->argv[0]);
				return -1;
				}
		
				}
	else if( n_commands > 1 ) pipe_commands(n_commands, com);
				
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
void pipe_commands( int n_commands, struct single_command (*com) ) {
	int pid, pid2, status;
	pthread_t th;
	pthread_create(&th, NULL, pipe_server, com);
	
	pid = fork();
	if( pid == 0 ) {
		//synchronization problem...
		sleep(1);
		int client_socket;
		struct sockaddr_un server_sockaddr;
		char buffer[BUFF_SIZE+5];
		
		client_socket = socket(PF_FILE, SOCK_STREAM, 0);
		memset(&server_sockaddr, 0, sizeof(server_sockaddr));
		server_sockaddr.sun_family = AF_UNIX;
		strcpy(server_sockaddr.sun_path, SOCK_PATH);
	  	if( connect(client_socket, (struct sockaddr*)&server_sockaddr, sizeof(server_sockaddr)) == -1 ) {
			printf("Connection failed!!\n");
			exit(1);
		}
		pid2 = fork();
		if(pid2 == 0) {
			//Close the file descriptor stdin, stdout
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			if ( dup2(client_socket, STDOUT_FILENO) == -1  ) {
				printf("Connect Stdin to stdout is failed!!\n");
				exit(1);
			}		
			evaluate_command(1,com);
			exit(0);
		}
		wait(&status);
		close(client_socket);
		exit(0);
	}
	else wait(&status);
	pthread_join(th,  NULL);
}
void *pipe_server(struct single_command (*com)) {
	struct single_command data[512] = {};
	int server_socket, client_socket, client_size, status, rc;
	struct sockaddr_un server_sockaddr;
	struct sockaddr_un client_sockaddr;
	char buff_server[BUFF_SIZE+5], buff_client[BUFF_SIZE+5];
	
	memcpy(data,com+1,sizeof(struct single_command)*511);

	if( access(SOCK_PATH, F_OK ) == 0  ) unlink(SOCK_PATH);
	server_socket = socket(PF_FILE, SOCK_STREAM, 0);
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));
	server_sockaddr.sun_family = AF_UNIX;
	strcpy(server_sockaddr.sun_path, SOCK_PATH);
		
	rc = bind(server_socket, (struct sockaddr*)&server_sockaddr,sizeof(server_sockaddr));
	if( rc == -1 ) {
		printf("Bind error occured!!\n");
		close(server_socket);
		exit(1);
	}
	while(1) {
		if ( listen(server_socket,5) == -1  ) {
			printf("Listen error occured!!\n");
			exit(1);
		}
		client_socket = accept(server_socket, (struct sockaddr*)&client_sockaddr, &client_size);
		assert(client_socket != -1);
		if( fork() == 0 ) {
			close(STDIN_FILENO);
			dup2(client_socket, STDIN_FILENO);
			evaluate_command(1, data);
			exit(0);  
		}
		wait(&status);
		close(client_socket);
		pthread_exit(0);
	}
}
//If the command contains "&" character, it retuns 1. If not, 0.
int isBack( struct single_command (*com), int n_commands  ) {
	int result = 0;
	for( int i = 0; i < n_commands; i++ ) {
		for( int j = 0; j < com[i].argc; j++  ) {
			if( strcmp(com[i].argv[j],"&") == 0 ) result++;
		}
		if( result > 0 ) break;
	}
	return result;
}
