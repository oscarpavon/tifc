#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <pty.h>

#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nCaught SIGINT (Ctrl+C). Custom handling here...\n");
}

// Configure terminal settings
void configure_terminal(int fd) {
    struct termios tty;
    tcgetattr(fd, &tty);
    tty.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tty.c_cc[VMIN] = 1; // Minimum number of characters to read
    tty.c_cc[VTIME] = 0; // No timeout
    tcsetattr(fd, TCSANOW, &tty); // Apply changes immediately
}

// Restore terminal settings
void restore_terminal(int fd) {
    struct termios tty;
    tcgetattr(fd, &tty);
    tty.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
    tcsetattr(fd, TCSANOW, &tty); // Apply changes immediately
}

int main() {
    int master_fd; // Pseudoterminal master file descriptor
    pid_t pid;

    // Fork a shell process using forkpty
    pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (pid == -1) {
        perror("forkpty");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: execute the shell
        execl("/bin/bash", "bash", NULL); // Replace with your shell
        perror("execl");
        exit(EXIT_FAILURE);
    }

    // Parent process: set up epoll
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // Configure terminal settings for stdin
    configure_terminal(STDIN_FILENO);

    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN; // Monitor for input
    ev.data.fd = master_fd; // Monitor the PTY master
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, master_fd, &ev) == -1) {
        perror("epoll_ctl: master_fd");
        exit(EXIT_FAILURE);
    }

    ev.data.fd = STDIN_FILENO; // Monitor standard input
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        perror("epoll_ctl: stdin");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];

    // Main loop to interact with the shell
    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (n == -1) {
            if (errno == EINTR) {
                // The call was interrupted by a signal; just continue
                continue; // Retry epoll_wait
            }
            perror("epoll_wait");
            break;
        }


        for (int i = 0; i < n; i++) {
            if (events[i].events & EPOLLIN) {
                if (events[i].data.fd == master_fd) {
                    // Data available from the shell
                    int bytes_read = read(master_fd, buffer, BUFFER_SIZE - 1);
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        printf("%s", buffer); // Print shell output
                        fflush(stdout);
                    } else if (bytes_read == 0) {
                        // Shell has closed
                        printf("\nShell process closed.\n");
                        goto cleanup;
                    } else {
                        perror("read from master_fd");
                    }
                } else if (events[i].data.fd == STDIN_FILENO) {
                    // Data available from stdin
                    int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
                    if (bytes_read > 0) {
                        // Check for Ctrl+D
                        if (memchr(buffer, '\x04', bytes_read)) {
                            printf("\nEOF detected. Exiting...\n");
                            goto cleanup;
                        }
                        if (write(master_fd, buffer, bytes_read) == -1) {
                            perror("write to master_fd");
                        }
                    } else if (bytes_read == -1) {
                        perror("read from stdin");
                    }
                }
            }
        }
    }

cleanup:
    restore_terminal(STDIN_FILENO); // Restore terminal settings
    close(master_fd);
    close(epfd);
    wait(NULL); // Wait for the child process to finish
    return 0;
}

