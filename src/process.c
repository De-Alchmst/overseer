#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "process.h"
#include "intercept.h"
#include "quit.h"

// TODO: implement TRACEGOOD
void* handleProcess(void* args) {
    pid_t pid = *((pid_t*)args);

	int status = 0;
	int entering = 0;
	struct user_regs_struct regs;

    printf("Attaching to process %d...\n", pid);
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) != -1) {
        while (!quit) {
            if (waitpid(pid, &status, __WALL) < 0) {
                perror("waitpid failed");
                break;
            }

            if (WIFEXITED(status)) {
                printf("Child exited with status %d\n", WEXITSTATUS(status));
                break;
            }

            if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) == -1) {
                perror("ptrace PTRACE_GETREGS failed");
                break;
            }

            handleSyscall(pid, &regs, &entering);

            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) {
                perror("ptrace PTRACE_SYSCALL failed");
                break;
            }

            entering = !entering;
        }
    }

    return NULL;
}
