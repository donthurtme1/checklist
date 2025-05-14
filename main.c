/*
 * Simple program will read todo-list and turn it into
 * a simple terminal check-list.
 * 
 * Use j/k to move the cursor up/down,
 * Space to toggle the current item and
 * Enter/Backspace to mark the current item as either
 * completed/not completed respectively.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define die(msg) { perror(msg); exit(1); }

void
print_checklist(char *checklist[], char completed[])
{
	/* Save cursor position */
	printf("\e[s");

	for (int i = 0; checklist[i] != NULL; i++)
	{
		printf("\e[%d;1H%s:\e[22G[%c]\n\n", (2 * i) + 1, checklist[i], completed[i]);
	}

	/* Load cursor position */
	printf("\e[u");
}

int
main(int argc, char *argv[])
{
	/*
	 * Initialise variables
	 */
	char *homedir_str = getenv("HOME");
	char *todolist_str = malloc(strlen(homedir_str) + sizeof("/.todo"));
	if (todolist_str == NULL)
		die("malloc");
	strcpy(todolist_str, homedir_str);
	strcat(todolist_str, "/.todo");

	FILE *todolist_fp = fopen(todolist_str, "r");
	free(todolist_str);
	if (todolist_fp == NULL)
		die("fopen");

	static char *todolist[64] = { NULL };
	char *line = NULL;
	size_t size = 0;
	int n_list_items = 0;
	for (long len = 0; /* NOTE: Not the counter */
			(len = getline(&line, &size, todolist_fp)) > 0 && n_list_items < 64;
			n_list_items++)
	{
		todolist[n_list_items] = malloc(len);
		strcpy(todolist[n_list_items], line);
		todolist[n_list_items][len - 1] = '\0';
	}

	static char completed[64];
	memset(completed, ' ', 64);

	/*
	 * Initialise terminal
	 */
	printf("\e[?1049h");

	struct termios old_term, new_term;
	tcgetattr(1, &old_term);
	new_term = old_term;
	new_term.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(1, 0, &new_term);

	printf("\e[23G");
	print_checklist(todolist, completed);

	/*
	 * Main loop
	 */
	int list_idx = 0;
	char c = 0;
	while ((c = getc(stdin)) != 'q')
	{
		switch (c)
		{
			case 'k': /* Up */
				if (list_idx <= 0)
					continue;
				list_idx--;
				printf("\e[2A");
				continue;
			case 'j': /* Down */
				if (list_idx >= n_list_items - 1)
					continue;
				list_idx++;
				printf("\e[2B");
				continue;

			case 0x7f: /* Set uncompleted */
				completed[list_idx] = ' ';
				if (list_idx > 0) {
					list_idx--;
					printf("\e[2A");
				}
				break;
			case '\n': /* Set completed */
				completed[list_idx] = 'x';
				if (list_idx < n_list_items - 1) {
					list_idx++;
					printf("\e[2B");
				}
				break;

			case ' ': /* Toggle completed */
				completed[list_idx] ^= ('x' ^ ' ');
				break;
		}

		print_checklist(todolist, completed);
	}

	/*
	 * Cleanup terminal
	 */
	tcsetattr(1, 0, &old_term);
	printf("\e[?1049l");

	return 0;
}
