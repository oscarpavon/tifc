#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nCaught SIGINT (Ctrl+C). Custom handling here...\n");
    // Optionally, you could set a flag or perform custom actions here
    // e.g., safely close resources or request termination
}

int main() {
    // Register the signal handler for SIGINT
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask); // Do not block any other signals

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Main loop to interact with the shell
    while (1) {
        printf("Running... Press Ctrl+C to trigger custom handler.\n");
        sleep(1); // Simulate work, replace with actual functionality
    }

    return 0;
}

