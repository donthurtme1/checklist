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
print_checklist(char *const checklist[], const char completed[], const int list_idx)
{
	/* Save cursor position */
	printf("\e[s");
	if (list_idx > 0)
		printf("\e[%dA", list_idx);

	for (int i = 0; checklist[i] != NULL; i++) {
		printf("\e[G%s:\e[25G[%c]\e[B", checklist[i], completed[i]);
	}

	/* Load cursor position */
	printf("\e[u");
}

int
main(int argc, char *argv[])
{
	/*
	 * Command line arguments
	 */
	static struct options {
		int completed_char_utf8;
		int alternate_buffer;
	} opt = {
		.completed_char_utf8 = 'x',
		.alternate_buffer = 0,
	};

	for (int i = 0;
			argv[i] != NULL;
			i++)
	{
		if (strcmp(argv[i], "-a") == 0)
		{
			i++;
			//add_item(argv[i]);
			exit(0);
		}
		else if (strncmp(argv[i], "--completed_char=", 17) == 0 &&
				argv[i][17] != '\0')
		{
			opt.completed_char_utf8 = argv[i][17];
		}
		else if (strcmp(argv[i], "--alternate_buffer") == 0)
		{
			opt.alternate_buffer = 1;
		}
	}

	/*
	 * Initialise variables
	 */
	char *homedir_str = getenv("HOME");
	char *todolist_str = malloc(strlen(homedir_str) + sizeof("/.todo"));
	char *todo_cache_str = malloc(strlen(homedir_str) + sizeof("/.cache/todo"));
	if (todolist_str == NULL || todo_cache_str == NULL)
		die("malloc");

	strcpy(todolist_str, homedir_str);
	strcat(todolist_str, "/.todo");
	strcpy(todo_cache_str, homedir_str);
	strcat(todo_cache_str, "/.cache/todo");

	FILE *todolist_fp = fopen(todolist_str, "r");
	FILE *todo_cache_fp = fopen(todo_cache_str, "rw");
	free(todolist_str);
	free(todo_cache_str);
	if (todolist_fp == NULL || todo_cache_fp == NULL)
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
	if (opt.alternate_buffer)
		printf("\e[?1049h");
	else
	{
		for (int i = 0; i < n_list_items; i++)
			putc('\n', stdout);
		printf("\e[%dA", n_list_items);
	}

	struct termios old_term, new_term;
	tcgetattr(1, &old_term);
	new_term = old_term;
	new_term.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(1, 0, &new_term);

	printf("\e[26G");
	print_checklist(todolist, completed, 0);

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
				if (list_idx > 0) {
					list_idx--;
					printf("\e[A");
				}
				else {
					list_idx = n_list_items - 1;
					printf("\e[%dB", list_idx);
				}
				continue;
			case 'j': /* Down */
				if (list_idx < n_list_items - 1) {
					list_idx++;
					printf("\e[B");
				}
				else {
					list_idx = 0;
					printf("\e[%dA", n_list_items - 1);
				}
				continue;

			case 0x7f: /* Set uncompleted */
				completed[list_idx] = ' ';
				if (list_idx > 0) {
					list_idx--;
					printf("\e[A");
				}
				break;
			case '\n': /* Set completed */
				completed[list_idx] = opt.completed_char_utf8;
				if (list_idx < n_list_items - 1) {
					list_idx++;
					printf("\e[B");
				}
				break;

			case ' ': /* Toggle completed */
				completed[list_idx] ^= (opt.completed_char_utf8 ^ ' ');
				break;
		}

		print_checklist(todolist, completed, list_idx);
	}

	/*
	 * Cleanup terminal
	 */
	tcsetattr(1, 0, &old_term);
	if (opt.alternate_buffer)
		printf("\e[?1049l");
	else
		printf("\e[%dB\e[G", n_list_items - list_idx);

	return 0;
}
