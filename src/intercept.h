#include <sys/user.h>

void handleSyscall(pid_t pid, struct user_regs_struct* regs, int* entering);
