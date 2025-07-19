#include <stdio.h>
#include <stdlib.h>
#include <sys/user.h>
#include <unistd.h>
#include <pthread.h>

#include "launch.h"
#include "process.h"
#include "quit.h"

void handleChild(char* user, char* command, char* args[]);
void handleParent(pid_t pid);

// Only used to launch the initial process
void launch(char* user, char* command, char* args[]) {
	pid_t pid = fork();

	if (pid < 0) {
		perror("Fork failed");
		exit(1);
	}

	if (pid == 0) {
		handleChild(user, command, args);
	} else {
		handleParent(pid);
	}
}


void handleChild(char* user, char* command, char* args[]) {
	execvp(command, args);
	perror("execvp failed");
	exit(1);
}


void handleParent(pid_t pid) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handleProcess, (void*)&pid);

    // terminate after the main process quits 
    pthread_join(thread_id, NULL);
    quit = 1;
}
