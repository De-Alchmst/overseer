#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "intercept.h"

#ifdef __x86_64__
#define SYSCALL_CODE(regs) ((regs)->orig_rax)
#define ARG1(regs)         ((regs)->rdi     )
#define ARG2(regs)         ((regs)->rsi     )
#define ARG3(regs)         ((regs)->rdx     )
#define ARG4(regs)         ((regs)->r10     )
#define ARG5(regs)         ((regs)->r8      )
#define ARG6(regs)         ((regs)->r9      )
#define RETURN_VALUE(regs) ((regs)->rax     )

#define CODE_INVALID -38

#define CODE_EXECVE 59

#define CODE_FORK   57
#define CODE_VFORK  58

#define CODE_CLONE  56
#define CODE_CLONE3 435
#endif

// TODO: handle CLONE_THREAD

void handleSyscall(pid_t pid, struct user_regs_struct* regs, int* entering) {
    if (!*entering && (SYSCALL_CODE(regs) == CODE_FORK  ||
                       SYSCALL_CODE(regs) == CODE_VFORK ||
                       SYSCALL_CODE(regs) == CODE_CLONE ||
                       SYSCALL_CODE(regs) == CODE_CLONE3)) {

        if (RETURN_VALUE(regs) == CODE_INVALID) {
            *entering = !*entering;
            return;
        }
        printf("Intercepted fork syscall:\n");
        printf("  PID: %d\n", pid);
        printf("  Return value: %lld\n", RETURN_VALUE(regs));
    }

    // if (entering) {
    //     if (SYSCALL_CODE(regs) == CODE_EXECVE) {
    //         printf("Intercepted execve syscall:\n");
    //         printf("  PID: %d\n", pid);
    //         printf("  Command: %s\n", (char*)ARG1(regs));
    //         printf("  Command: %s\n", (char*)ARG3(regs));
    //         printf("  Command: %s\n", (char*)ARG2(regs));
    //         printf("  Command: %lld\n", ARG4(regs));
    //         printf("  Command: %lld\n", ARG5(regs));
    //         printf("  Command: %lld\n", ARG6(regs));
    //     }
    // }
}
