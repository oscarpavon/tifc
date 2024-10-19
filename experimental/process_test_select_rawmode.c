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

// Function to set terminal to raw mode
void set_raw_mode(int fd) {
    struct termios term;
    if (tcgetattr(fd, &term) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    term.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    term.c_cc[VMIN] = 1; // Minimum number of characters for non-canonical read
    term.c_cc[VTIME] = 0; // Timeout for non-canonical read

    if (tcsetattr(fd, TCSANOW, &term) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

// Function to restore the terminal to its original mode
void restore_terminal_mode(int fd, struct termios *orig_term) {
    if (tcsetattr(fd, TCSANOW, orig_term) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int master_fd;
    pid_t pid;
    struct winsize ws;
    char buffer[BUFFER_SIZE];
    struct termios orig_term;

    // Save the original terminal settings
    if (tcgetattr(STDIN_FILENO, &orig_term) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Set terminal to raw mode
    set_raw_mode(STDIN_FILENO);

    // Set desired terminal size
    ws.ws_col = 40; // Number of columns
    ws.ws_row = 1; // Number of rows
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;

    // Fork a new process with a pseudoterminal
    pid = forkpty(&master_fd, NULL, NULL, &ws);
    if (pid == -1) {
        perror("forkpty");
        restore_terminal_mode(STDIN_FILENO, &orig_term);
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
                } else if (n == -1) {
                    if (errno == EINTR || errno == EAGAIN) {
                        // Interrupted system call or resource temporarily unavailable, continue the loop
                        continue;
                    } else {
                        perror("read from master_fd");
                        break;
                    }
                } else if (n == 0) {
                    // End of file (EOF) on the pseudoterminal
                    printf("\nShell process closed.\n");
                    FD_CLR(master_fd, &read_fds); // Unregister from select
                    break;
                }
            }

            // Check if there's data to read from stdin
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                int n = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
                if (n > 0) {
                    // Check if the input contains the Ctrl+D character
                    if (memchr(buffer, '\x04', n)) {
                        printf("\nEOF detected. Exiting...\n");
                        FD_CLR(STDIN_FILENO, &read_fds); // Unregister from select
                        break;
                    }
                    buffer[n] = '\0';
                    // Write user input to the pseudoterminal
                    if (write(master_fd, buffer, n) == -1) {
                        perror("write to master_fd");
                        break;
                    }
                } else if (n == -1) {
                    if (errno == EINTR || errno == EAGAIN) {
                        // Interrupted system call or resource temporarily unavailable, continue the loop
                        continue;
                    } else {
                        perror("read from stdin");
                        break;
                    }
                }
            }
        }


        // Restore the original terminal settings
        restore_terminal_mode(STDIN_FILENO, &orig_term);

        // Close the master file descriptor
        close(master_fd);

        // Wait for the child process to finish
        wait(NULL);
    }

    return 0;
}

