#ifndef MYSH_COMMANDS_H_
#define MYSH_COMMANDS_H_

struct single_command
{
  int argc;
  char** argv;
};

int evaluate_command(int n_commands, struct single_command (*commands)[512]);

void free_commands(int n_commands, struct single_command (*commands)[512]);

void *pipe_server(struct single_command (*com));
void pipe_commands(int n_commands, struct single_command (*com));
int isBack( struct single_command (*com), int n_commands );

#endif // MYSH_COMMANDS_H_
