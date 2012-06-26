#include <stdio.h>
#include <string.h>

#include "fifo-wm.h"
#include "commands.h"
#include "tree.h"

void handleCommand(char* request) {
	fprintf(stderr, "Recv from FIFO: %s", request);

	//Array to contain pointers to tokens, max of 5 tokens
	char *tokens[5];

	int counter = 0;
	char *lastToken, *token;
	for ( token = strtok_r(request, " ", &lastToken); token; token = strtok_r(NULL, " ", &lastToken)) {
		tokens[counter++] = token;
		fprintf(stderr, "Adding Token %s\n", token);
	}


	//fprintf(stderr, "Last Token %s");

	if (!strcmp(tokens[0], "kill")) {
		fprintf(stderr, "Killing Client");
	} else if (!strcmp(tokens[0], "dump")) {
		dumpTree();
	} else if (!strcmp(tokens[0], "layout")) {
		fprintf(stderr, "Setting layout to: %s", tokens[1]);
		if (!strcmp(tokens[1], "vertical")) {	
			lastContainer -> layout = 0;
		} else if (!strcmp(tokens[1], "horizontal")) {
			lastContainer -> layout = 1;
		}
	} else if (!strcmp(tokens[0], "spawn")) {
		if (!strcmp(tokens[1], "child")) {
			fprintf(stderr, "Setting spawn to child");
			spawn = 1;
		} else if (!strcmp(tokens[1], "brother")) {
			fprintf(stderr, "Setting spawn to brother");
			spawn = 0;
		}
	}
}
