#include <stdio.h>
#include <sys/user.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "intercept.h"
#include "process.h"

// TODO: ARM support
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

// Will attach new handleProcess to new processes created by fork, vfork, clone,
// or clone3 syscalls.
void handleSyscall(pid_t pid, struct user_regs_struct* regs, int* entering) {
    if (!*entering && (SYSCALL_CODE(regs) == CODE_FORK  ||
                       SYSCALL_CODE(regs) == CODE_VFORK ||
                       SYSCALL_CODE(regs) == CODE_CLONE ||
                       SYSCALL_CODE(regs) == CODE_CLONE3)) {

        pid_t* child_pid = malloc(sizeof(pid_t));
        *child_pid = RETURN_VALUE(regs);

        // invalid syscall, just grab the next one
        if (*child_pid == CODE_INVALID) {
            *entering = !*entering;
            return;
        }

        printf("Intercepted syscall %lld, child pid: %d\n", SYSCALL_CODE(regs), *child_pid);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleProcess, (void*)child_pid);
        pthread_detach(thread_id);
    }
}
