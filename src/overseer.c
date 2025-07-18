#include <stdio.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [flags]\n", argv[0]);
        return 1;
    }

    // Simulate command execution
    printf("Executing command: %s\n", argv[1]);

    // Here you would add the logic to handle the command
    // For now, we just print a success message
    printf("Command '%s' executed successfully.\n", argv[1]);

    return 0;
}
