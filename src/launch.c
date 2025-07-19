#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#include "launch.h"
#include "process.h"

void handleChild(char* user, char* command, char* args[]);
void handleParent(pid_t pid);

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
    printf("Parent process: %d, waiting for child process: %d\n", getpid(), pid);
    pthread_create(&thread_id, NULL, handleProcess, (void*)&pid);
    // handleProcess((void*)&pid);
}
