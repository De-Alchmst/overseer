package main

import (
    "fmt"
    "syscall"
    "golang.org/x/sys/unix"
)

type SyscallInterceptor struct {
    pid int
}

func NewSyscallInterceptor(pid int) *SyscallInterceptor {
    return &SyscallInterceptor{pid: pid}
}

func (si *SyscallInterceptor) getRegisters() (*unix.PtraceRegs, error) {
    var regs unix.PtraceRegs
    err := unix.PtraceGetRegs(si.pid, &regs)
    return &regs, err
}

func (si *SyscallInterceptor) setRegisters(regs *unix.PtraceRegs) error {
    return unix.PtraceSetRegs(si.pid, regs)
}

func (si *SyscallInterceptor) readString(addr uintptr, maxLen int) (string, error) {
    data := make([]byte, maxLen)
    n, err := unix.PtracePeekData(si.pid, addr, data)
    if err != nil {
        return "", err
    }
    
    // Find null terminator
    for i := 0; i < n; i++ {
        if data[i] == 0 {
            return string(data[:i]), nil
        }
    }
    return string(data[:n]), nil
}

func (si *SyscallInterceptor) writeString(addr uintptr, str string) error {
    data := []byte(str + "\x00") // Add null terminator
    _, err := unix.PtracePokeData(si.pid, addr, data)
    return err
}

func (si *SyscallInterceptor) interceptOpen(regs *unix.PtraceRegs) error {
    // Read the filename argument (first argument)
    filename, err := si.readString(uintptr(getArg1(regs)), 256)
    if err != nil {
        return err
    }
    
    fmt.Printf("INTERCEPT: open(\"%s\", %d, %d)\n", filename, getArg2(regs), getArg3(regs))
    
    // Example: Block access to /etc/passwd
    if filename == "/etc/passwd" {
        fmt.Println("BLOCKED: Access to /etc/passwd denied!")
        // Set return value to -1 (error) and errno to EACCES
        si.setReturnValue(regs, uint64(syscall.EACCES))
        return si.setRegisters(regs)
    }
    
    return nil
}

func (si *SyscallInterceptor) interceptOpenat(regs *unix.PtraceRegs) error {
    // Read the filename argument (second argument)
    filename, err := si.readString(uintptr(getArg2(regs)), 256)
    if err != nil {
        return err
    }
    
    fmt.Printf("INTERCEPT: openat(%d, \"%s\", %d, %d)\n", getArg1(regs), filename, getArg3(regs), getArg4(regs))
    
    // Example: Redirect file access
    if filename == "/tmp/secret.txt" {
        fmt.Println("REDIRECT: Redirecting to /tmp/dummy.txt")
        // Write new filename to a scratch area in memory
        // This is simplified - in practice you'd need to allocate memory properly
        newPath := "/tmp/dummy.txt"
        if err := si.writeString(uintptr(getArg2(regs)), newPath); err != nil {
            return err
        }
    }
    
    return nil
}

func (si *SyscallInterceptor) interceptWrite(regs *unix.PtraceRegs) error {
    // Read the buffer content (second argument, length in third argument)
    bufLen := int(getArg3(regs))
    if bufLen > 1024 {
        bufLen = 1024 // Limit reading to prevent issues
    }
    
    data := make([]byte, bufLen)
    _, err := unix.PtracePeekData(si.pid, uintptr(getArg2(regs)), data)
    if err != nil {
        return err
    }
    
    // Find actual length (up to first null byte or full buffer)
    actualLen := bufLen
    for i, b := range data {
        if b == 0 {
            actualLen = i
            break
        }
    }
    
    content := string(data[:actualLen])
    fmt.Printf("INTERCEPT: write(%d, \"%s\", %d)\n", getArg1(regs), content, getArg3(regs))
    
    // Example: Filter out profanity
    if containsProfanity(content) {
        fmt.Println("FILTER: Profanity detected, replacing with [CENSORED]")
        censored := "[CENSORED]"
        censoredBytes := []byte(censored)
        _, err := unix.PtracePokeData(si.pid, uintptr(getArg2(regs)), censoredBytes)
        if err != nil {
            return err
        }
        // Update the length
        si.setArg3(regs, uint64(len(censored)))
        return si.setRegisters(regs)
    }
    
    return nil
}

func containsProfanity(content string) bool {
    // Simple profanity filter - you'd use a more sophisticated one
    profanity := []string{"damn", "hell", "shit"}
    for _, word := range profanity {
        if contains(content, word) {
            return true
        }
    }
    return false
}

func contains(s, substr string) bool {
    return len(s) >= len(substr) && 
           (s == substr || 
            (len(s) > len(substr) && (s[:len(substr)] == substr || contains(s[1:], substr))))
}

func (si *SyscallInterceptor) handleSyscall(regs *unix.PtraceRegs, entering bool) error {
    syscallNum := getSyscallNum(regs)
    
    if entering {
        fmt.Printf("ENTERING syscall %d\n", syscallNum)
        
        switch syscallNum {
        case unix.SYS_OPEN:
            return si.interceptOpen(regs)
        case unix.SYS_OPENAT:
            return si.interceptOpenat(regs)
        case unix.SYS_WRITE:
            return si.interceptWrite(regs)
        default:
            // Don't intercept other syscalls
            return nil
        }
    } else {
        retVal := si.getReturnValue(regs)
        fmt.Printf("EXITING syscall %d, return value: %d\n", syscallNum, int64(retVal))
        
        // Handle syscall exit if needed
        switch syscallNum {
        case unix.SYS_OPEN, unix.SYS_OPENAT:
            if int64(retVal) >= 0 {
                fmt.Printf("File descriptor: %d\n", retVal)
            } else {
                fmt.Printf("Error: %d\n", int64(retVal))
            }
        }
    }
    
    return nil
}
