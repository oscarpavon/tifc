#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/select.h>

#define BUFFER_SIZE 256

int main() {
    int master_fd;
    pid_t pid;
    struct winsize ws;
    char buffer[BUFFER_SIZE];

    // Set desired terminal size
    ws.ws_col = 80; // Number of columns
    ws.ws_row = 24; // Number of rows
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    // Fork a new process with a pseudoterminal
    pid = forkpty(&master_fd, NULL, NULL, &ws);
    if (pid == -1) {
        perror("forkpty");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { // Child process
        // Execute the shell
        execlp("/bin/sh", "sh", NULL);
        // If exec fails
        perror("execlp");
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Main loop to interact with the shell
        while (1) {
            // Use select to wait for input on either master_fd or stdin
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(master_fd, &read_fds); // Watch the pseudoterminal
            FD_SET(STDIN_FILENO, &read_fds); // Watch stdin

            int max_fd = master_fd > STDIN_FILENO ? master_fd : STDIN_FILENO;
            int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
            if (ready == -1) {
                perror("select");
                break;
            }

            // Check if there's data to read from the pseudoterminal
            if (FD_ISSET(master_fd, &read_fds)) {
                int n = read(master_fd, buffer, BUFFER_SIZE - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    printf("%s", buffer);
                    fflush(stdout);
                } else if (n == -1 && errno != EAGAIN) {
                    perror("read from master_fd");
                    break;
                } else if (n == 0) {
                    // End of file
                    break;
                }
            }

            // Check if there's data to read from stdin
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                int n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    // Write user input to the pseudoterminal
                    if (write(master_fd, buffer, n) == -1) {
                        perror("write to master_fd");
                        break;
                    }
                } else if (n == -1 && errno != EAGAIN) {
                    perror("read from stdin");
                    break;
                }
            }
        }

        // Close the master file descriptor
        close(master_fd);

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}

