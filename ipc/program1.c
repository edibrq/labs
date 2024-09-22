#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

void read_and_write_to_pipe(const char *file_name, int write_fd) {
  int file_fd = open(file_name, O_RDONLY); // open file to read
  if (!file_fd) {
    printf("Can't open file %s\n", file_name);
    exit(EXIT_FAILURE);
  }

  // reading file content
  char buffer[1024];
  size_t bytes_read = 0;
  while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
    // writing to the stdout
    write(write_fd, buffer, bytes_read);
  }
  
  // closing resources
  close(file_fd);
  close(write_fd);
}

int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Provide name of the file\n");
    exit(EXIT_FAILURE);
  }
  
  // Create a file descriptors
  // fd[0] - to read
  // fd[1] - to write
  int fd[2];
  pipe(fd);
  char *file_name = argv[1];
  
  // creating new process to read the file content
  int process_id = fork();
  if (process_id == -1) {
    printf("Failed to start new process.\n");
    exit(EXIT_FAILURE);
  }
  
  if (process_id == 0) {
    // we are in child process
    close(fd[0]); // close fd to read
    read_and_write_to_pipe(file_name, fd[1]);
    exit(EXIT_SUCCESS);
  } else {
    // we are in parent process
    // so we have to read data from child process
    close(fd[1]);
    char buffer[BUFFER_SIZE];
    size_t bytes_read = 0;
    while ((bytes_read = read(fd[0], buffer, sizeof(buffer))) > 0) {
      write(STDOUT_FILENO, buffer, bytes_read);
    }
    close(fd[0]);
  }
  return EXIT_SUCCESS;
}
