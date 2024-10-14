#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1024

void execute_program(char *file_name, int write_fd) {
    if (fork() == 0) {
        dup2(write_fd, STDOUT_FILENO);
        execl("./program1", "program1", file_name, NULL);
        perror("execl Failed to run program1\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: program2.c <file1> <file2> <output_file>\n");
        exit(EXIT_FAILURE);
    }

    int fd_prog1[2], fd_prog2[2];
    pipe(fd_prog1);
    pipe(fd_prog2);

    char *file_name1 = argv[1];
    char *file_name2 = argv[2];
    execute_program(file_name1, fd_prog1[1]);
    execute_program(file_name2, fd_prog2[1]);
    close(fd_prog1[1]);
    close(fd_prog2[1]);

    char *output_file = argv[3];
    int output_file_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // Читаем весь второй файл в память
    char *file2_data = NULL;
    size_t file2_size = 0;
    size_t file2_capacity = BUFFER_SIZE;
    file2_data = (char*) malloc(file2_capacity);
    if (!file2_data) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd_prog2[0], file2_data + file2_size, BUFFER_SIZE)) > 0) {
        file2_size += bytes_read;
        if (file2_size + BUFFER_SIZE > file2_capacity) {
            file2_capacity *= 2;
            file2_data = (char*) realloc(file2_data, file2_capacity);
            if (!file2_data) {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (file2_size == 0) {
        fprintf(stderr, "Warning: file2 is empty\n");
        free(file2_data);
        file2_data = (char*) malloc(1);
        file2_data[0] = 0;
        file2_size = 1;
    }

    char buffer1[BUFFER_SIZE];
    size_t file2_index = 0;

    while ((bytes_read = read(fd_prog1[0], buffer1, BUFFER_SIZE)) > 0) {
        for (size_t i = 0; i < bytes_read; i++) {
            buffer1[i] ^= file2_data[file2_index];
            file2_index = (file2_index + 1) % file2_size;
        }
        write(output_file_fd, buffer1, bytes_read);
    }

    close(fd_prog1[0]);
    close(fd_prog2[0]);
    close(output_file_fd);
    free(file2_data);

    return EXIT_SUCCESS;
}

