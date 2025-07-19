#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "process.h"
#include "intercept.h"

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
	if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
		perror("ptrace failed");
		exit(1);
	}

	execvp(command, args);
	perror("execvp failed");
	exit(1);
}


void handleParent(pid_t pid) {
	int status = 0;
	int entering = 0;
	struct user_regs_struct regs;

	for (;;) {
		if (waitpid(pid, &status, 0) < 0) {
			perror("waitpid failed");
			exit(1);
		}

		if (WIFEXITED(status)) {
			printf("Child exited with status %d\n", WEXITSTATUS(status));
			break;
		}

		if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
			perror("ptrace PTRACE_GETREGS failed");
			exit(1);
		}

        handleSyscall(pid, &regs, &entering);

		if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) {
			perror("ptrace PTRACE_SYSCALL failed");
			exit(1);
		}

		entering = !entering;
	}
}
