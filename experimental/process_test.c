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

#define BUFFER_SIZE 256

// Function to set a file descriptor to non-blocking mode
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        exit(EXIT_FAILURE);
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
        exit(EXIT_FAILURE);
    }
}

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
        execlp("/bin/fish", "fish", NULL);
        // If exec fails
        perror("execlp");
        exit(EXIT_FAILURE);
    } else { // Parent process
        // Set the pseudoterminal master file descriptor to non-blocking mode
        set_nonblocking(master_fd);
        // Set stdin to non-blocking as well
        set_nonblocking(STDIN_FILENO);

        // Loop to interact with the shell
        while (1) {
            // Read from the pseudoterminal
            int n = read(master_fd, buffer, BUFFER_SIZE - 1);
            if (n > 0) {
                buffer[n] = '\0';
                printf("%s", buffer);
                fflush(stdout);
            } else if (n == -1 && errno != EAGAIN) {
                perror("read from master_fd");
                break;
            }

            // Read from stdin
            n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
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

            // Small delay to avoid busy waiting
            usleep(10); // 10 ms
        }

        // Close the master file descriptor
        close(master_fd);

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}

