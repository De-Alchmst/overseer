package main

import (
    "fmt"
		"flag"
    "os"
		"os/user"
		"path"
)

var (
	userName *string
	exitChan chan int
)

// usage gives a basic overview of the command usage
func usage() {
	fmt.Println("Usage: overseer [flags] <command> [args...]")
	flag.PrintDefaults()
}


func main() {
	// Just for develepment purposes
	selfPath, err := os.Executable()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error getting executable path: %v\n", err)
		os.Exit(1)
	}
	os.Setenv("PATH", os.Getenv("PATH") + ":" + path.Dir(selfPath))

	currentUser, err := user.Current()
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error getting current user: %v\n", err)
		os.Exit(1)
	}

	flag.Usage = usage
	userName = flag.String("user", currentUser.Username, "User to run programs as (default: current user)")
	flag.Parse()

	if flag.NArg() < 1 {
		fmt.Fprintln(os.Stderr, "Error: Not enough arguments")
		flag.Usage()
		os.Exit(1)
	}

	fmt.Println("Running command as user:", *userName)

	launch(flag.Args())

	// Wait for a signal to exit
	// at least for now...
	_ = <-exitChan
}
