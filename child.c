#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>

void print_error(const char *msg, int len) {
    write(2, msg, len);
    exit(1);
}

long long check_prime(long long number) {
    if (number < 2) return 0;
    for (long long i = 2; i * i <= number; i++) {
        if (number % i == 0) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) print_error("child.c got less than 2 arguments. :(\n", 37);

    char *filename = argv[1];
    int pipe2fd = 1;

    int filefd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (filefd == -1) print_error("File not opened. :(\n", 19);

    long long number = 0;
    int negative = 0;
    int has_digits = 0;
    char num;

    while (read(0, &num, 1) > 0) {
        if (num == '-') {
            negative = 1;
        } else if (num >= '0' && num <= '9') {
            number = number * 10 + (num - '0');
            has_digits = 1;
        } else if (num == ' ' || num == '\n' || num == '\r') {
            if (has_digits) {
                if (negative) {
                    number = -number;
                }

                if (number <= 0 || number == 1 || check_prime(number)) {
                    char signal = 'X';
                    write(pipe2fd, &signal, 1);
                    close(filefd);
                    close(pipe2fd);
                    exit(0);
                } else {
                    char temp[32];
                    char buffer[32];
                    int t_pos = 0;
                    long long n = number;

                    while (n > 0) {
                        temp[t_pos++] = '0' + (n % 10);
                        n /= 10;
                    }
                    int b_pos = 0;
                    while (t_pos > 0) {
                        buffer[b_pos++] = temp[--t_pos];
                    }
                    buffer[b_pos++] = '\n';
                    write(filefd, buffer, b_pos);
                }
            }
            number = 0;
            negative = 0;
            has_digits = 0;
        }
    }
    if (has_digits) {
        if (negative) number = -number;
        if (number <= 0 || number == 1 || check_prime(number)) {
            char signal = 'X';
            write(pipe2fd, &signal, 1);
            close(filefd);
            close(pipe2fd);
            exit(0);
        } else {
        char temp[32];
        char buffer[32];
        int t_pos = 0;
        long long n = number;

        while (n > 0) {
            temp[t_pos++] = '0' + (n % 10);
            n /= 10;
        }
        int b_pos = 0;
        while (t_pos > 0) {
            buffer[b_pos++] = temp[--t_pos];
        }
        buffer[b_pos++] = '\n';
        write(filefd, buffer, b_pos);
        }
    }
    close(filefd);
    close(pipe2fd);
    return 0;
}


