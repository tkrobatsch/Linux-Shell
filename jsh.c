/*
lab8
Tom Krobatsch
April 7, 2019

jsh
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "fields.h"
#include "jval.h"
#include "dllist.h"
#include "jrb.h"

int main(int argc, char** argv) {
	//varibles
	char *prompt;
	IS is;
	int i, end, background;
	int pid, status;
	int fd_in, fd_out;
	char **newargv;
	char *infile;
	char *outfile;
	int in_flag, out_flag;

	//arg catch and exit
	if (argc > 2) {
		printf("usage: jsh3 [prompt]\n");
		exit(1);
	}

	//make prompt
	if (argc == 2) {
		if (strcmp(argv[1], "-") == 0) { prompt = ""; }
		else { prompt = argv[1]; }
	}
	else { prompt = "jsh: "; }

	is = new_inputstruct(NULL);
	//print prompt before loop
	printf("%s", prompt);

	while (get_line(is) >= 0) {
		//exit
		if (strcmp(is->text1, "exit\n") == 0) { exit(0); }
		if (is->NF != 0) {
			newargv = (char **)malloc(sizeof(char *) * is->NF);

			//set bg
			if (strcmp(is->fields[is->NF - 1], "&") == 0) { background = 1; }
			else { background = 0; }

			//process line
			end = 0;
			in_flag = 0;
			out_flag = 0;
			for (i = 0; i < is->NF - background; i++) {
				if (strcmp(is->fields[i], ">") == 0) {
					out_flag = 1;
					outfile = is->fields[++i];
				}
				else if (strcmp(is->fields[i], ">>") == 0) {
					out_flag = 2;
					outfile = is->fields[++i];
				}
				else if (strcmp(is->fields[i], "<") == 0) {
					in_flag = 1;
					infile = is->fields[++i];
				}
				else {
					newargv[end] = is->fields[i];
					end++;
				}
			}
			newargv[end++] = NULL;

			//process command
			pid = fork();
			//child
			if (pid == 0) {
				//redirection
				if (in_flag == 1) {
					fd_in = open(infile, O_RDONLY);
					if (fd_in < 0) {
						perror(newargv[0]);
						exit(1);
					}
					if (dup2(fd_in, 0) != 0) {
						perror(newargv[0]);
						exit(1);
					}
					close(fd_in);
				}
				if (out_flag == 1) {
					fd_out = open(outfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
					if (fd_out < 0) {
						perror(newargv[0]);
						exit(1);
					}
					if (dup2(fd_out, 1) != 1) {
						perror(newargv[0]);
						exit(1);
					}
					close(fd_out);
				}
				if (out_flag == 2) {
					fd_out = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
					if (fd_out < 0) {
						perror(newargv[0]);
						exit(1);
					}
					if (dup2(fd_out, 1) != 1) {
						perror(newargv[0]);
						exit(1);
					}
					close(fd_out);
				}
				execvp(newargv[0], newargv);
				perror(newargv[0]);
				exit(1);
			}
			//parent
			else {
				//wait if not in background mode
				if (!background) { while (wait(&status) != pid); }
			}
		}
		//print prompt
		printf("%s", prompt);
	}
}