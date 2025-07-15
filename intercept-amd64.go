//go:build amd64

package main

import (
    "golang.org/x/sys/unix"
)

// Architecture-specific register access
func getSyscallNum(regs *unix.PtraceRegs) uint64 {
	return regs.Orig_rax
}

func getArg1(regs *unix.PtraceRegs) uint64 {
	return regs.Rdi
}

func getArg2(regs *unix.PtraceRegs) uint64 {
	return regs.Rsi
}

func getArg3(regs *unix.PtraceRegs) uint64 {
	return regs.Rdx
}

func getArg4(regs *unix.PtraceRegs) uint64 {
	return regs.R10
}

func (si *SyscallInterceptor) setReturnValue(regs *unix.PtraceRegs, value uint64) {
	regs.Rax = value
}

func (si *SyscallInterceptor) setArg3(regs *unix.PtraceRegs, value uint64) {
	regs.Rdx = value
}

func (si *SyscallInterceptor) getReturnValue(regs *unix.PtraceRegs) uint64 {
	return regs.Rax
}
