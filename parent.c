#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <signal.h>

void print_error(const char *msg, int len) {
    write(2, msg, len);
    exit(1);
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    write(1, "Write a name of out file\n", 25);
    char filename[256];
    int pos = 0;
    char a;
    while (read(0, &a, 1) > 0 && a != '\n' && a != '\r') {
        filename[pos++] = a;
    }
    filename[pos] = '\0';
    write(1, "Now u can write numbers\n", 24);

    int pipe1[2];
    int pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        print_error("Pipe initialization failed. :(\n", 31);
    }

    pid_t pid = fork();
    if (pid == -1) print_error("Something gone wrong with fork. :(\n", 35);

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        if (dup2(pipe1[0], 0) == -1) print_error("Something gone wrong with dup2 (pipe1 in). :(\n", 46);
        if (dup2(pipe2[1], 1) == -1) print_error("Something gone wrong with dup2 (pipe2 out). :(\n", 47);
        close(pipe1[0]);
        close(pipe2[1]);



        char *args[] = {"./child", filename, NULL };
        execve("./child", args, NULL);

        print_error("Something gone wrong with execve. :(\n", 37);
    }

    else {
        char child_signal;
        close(pipe1[0]);
        close(pipe2[1]);
        char buffer[128];
        int status;
        while (1) {
            if (waitpid(pid, &status, WNOHANG) == pid) {
                ssize_t bytes_read = read(pipe2[0], &child_signal, 1);
                if (bytes_read > 0 && child_signal == 'X') {
                    write(1, "From parent: terminate signal 'X' reached. Exiting!\n", 52);

                } else {
                    write(1, "From parent: process stoped without signal. Exiting!\n", 53);
                }
                break;
            }
            ssize_t bytes_read = read(0, buffer, sizeof(buffer));
            if (bytes_read > 0) {
                write(pipe1[1], buffer, bytes_read);
            } else {
                close(pipe1[1]);
                pipe1[1] = -1;
                waitpid(pid, &status, 0);
                ssize_t s = read(pipe2[0], &child_signal, 1);
                if (s > 0 && child_signal == 'X') {
                    write(1, "From parent: terminate signal 'X' reached. Exiting!\n", 52);
                } else {
                    write(1, "Nothing to read. Bye!\n", 22);
                }
                break;
            }
        }

        if (pipe1[1] != -1) {
            close(pipe1[1]);
        }
        close(pipe2[0]);
    }
    return 0;
}