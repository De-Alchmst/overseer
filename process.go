package main


import (
	"os/exec"
	"syscall"
	"fmt"
	"os"
	"golang.org/x/sys/unix"
)


// launch starts a new process from a command
func launch(args []string) {
	// prepare command
	// add user to the command
	newArgs := make([]string, len(args)+1)
	newArgs[0] = *userName
	copy(newArgs[1:], args)

	cmd := exec.Command("ns-launch", newArgs...)
	// cmd := exec.Command(args[0], args[1:]...)
	cmd.SysProcAttr = &syscall.SysProcAttr{
			Ptrace: true,
	}

	// launch the process
	err := cmd.Start()
	if err != nil {
		fmt.Printf("Failed to start process: %v\n", err)
		os.Exit(1)
	}

	var waitStatus syscall.WaitStatus
	// Wait for the process to stop
	// This is important to catch every syscall
	_, err = syscall.Wait4(cmd.Process.Pid, &waitStatus, 0, nil)
	if err != nil {
		// might happen if no syacalls
		fmt.Printf("Wait4 failed: %v\n", err)
		return
	}

	fmt.Println("Process started with PID:", cmd.Process.Pid)
	go manageProcess(cmd.Process.Pid)
}


// manageProcess manages trunning process, intercepting it's system calls
func manageProcess(pid int) {
	// Set ptrace options for system call tracing
	// PTRACE_O_TRACESYSGOOD : easier to distinguish between syscall enter and exit events
	// PTRACE_O_EXITKILL : automatically kill the traced process if the tracer dies
	err := unix.PtraceSetOptions(pid, unix.PTRACE_O_TRACESYSGOOD | unix.PTRACE_O_EXITKILL)
	if err != nil {
		fmt.Printf("PtraceSetOptions failed: %v\n", err)
		os.Exit(1)
	}

	interceptor := NewSyscallInterceptor(pid)
	entering := true // Track whether we're entering or exiting a syscall

	var waitStatus syscall.WaitStatus

	for {
		// Continue until next system call
		if err := unix.PtraceSyscall(pid, 0); err != nil {
			fmt.Printf("PtraceSyscall failed: %v\n", err)
			break
		}

		// Wait for the process to stop
		_, err := syscall.Wait4(pid, &waitStatus, 0, nil)
		if err != nil {
			fmt.Printf("Wait4 failed: %v\n", err)
			break
		}

		if waitStatus.Exited() {
			fmt.Printf("Process %d exited with code %d\n", pid, waitStatus.ExitStatus())
			break
		}

		if waitStatus.Stopped() && waitStatus.StopSignal() == syscall.SIGTRAP|0x80 {
			// Get registers to examine the system call
			regs, err := interceptor.getRegisters()
			if err != nil {
				fmt.Printf("Failed to get registers: %v\n", err)
				entering = true
				continue
			}

			// Handle the system call
			if err := interceptor.handleSyscall(regs, entering); err != nil {
				fmt.Printf("Error handling syscall: %v\n", err)
			}

			// Toggle between entering and exiting
			entering = !entering
		}
	}
}
