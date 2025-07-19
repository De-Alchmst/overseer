#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "launch.h"
#include "quit.h"

int quit = 0;

void help();
void terminate(int signum);


int main(int argc, char *argv[]) {
    if (argc < 3) {
        help();
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
        help();
        return 0;
    }

    char* user    = argv[1]; 
    char* command = argv[2];
    printf("%d\n", quit);

    // on SIGTERM, just set quit to 1
    struct sigaction action = {
        .sa_handler = terminate,
        .sa_flags = 0
    };
    sigaction(SIGTERM, &action, NULL);

    launch(user, command, argv + 3);

    while (!quit) {}

    return 0;
}


void help() {
    printf("Usage: overseer <user> <command> [args...]\n");
}


void terminate(int signum) {
    quit = 1;
}
