#include <stdio.h>
#include <string.h>

#include "launch.h"
#include "quit.h"

int quit = 0;

void help();

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

    launch(user, command, argv + 3);

    while (!quit) {}

    return 0;
}


void help() {
    printf("Usage: overseer <user> <command> [args...]\n");
}
