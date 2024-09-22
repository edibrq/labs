#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

/*

+-------------------+      +-------------------+      +-------------------+
|   Program 1 (P1)  |      |   Program 1 (P2)  |      |   Parent Process  |
|                   |      |                   |      |                   |
|  Reads file1.txt  |      |  Reads file2.txt  |      |                   |
|                   |      |                   |      |                   |
|  Writes to pipe1  |      |  Writes to pipe2  |      |                   |
|  (FD: pipe_fd1[1])|      |  (FD: pipe_fd2[1])|      |                   |
+--------+----------+      +--------+----------+      +--------+----------+
         |                          |                          |
         |                          |                          |
         v                          v                          |
+--------+----------+      +--------+----------+               |
|      Pipe 1       |      |      Pipe 2       |               |
|                   |      |                   |               |
|  [Data from P1]   |      |  [Data from P2]   |               |
|                   |      |                   |               |
+--------+----------+      +--------+----------+               |
         |                          |                          |
         |                          |                          |
         v                          v                          v
+--------+--------------------------+--------------------------+
|                   Parent Process (Main Program)              |
|                                                              |
|  Reads from pipe1 (FD: pipe_fd1[0])                          |
|  Reads from pipe2 (FD: pipe_fd2[0])                          |
|  Performs XOR on data from both pipes                        |
|  Writes result to output file                                |
+--------------------------------------------------------------+

fd[0] - to read
fd[1] - to write

*/

void execute_program(char *file_name, int write_fd) {
  // create a child process
  if (fork() == 0) {
    // move stdout to the write_fd 
    dup2(write_fd, STDOUT_FILENO);
    // start program1 in a new process
    execl("./program1", "program1", file_name, NULL);
    perror("execl Failed to run program1\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  if (argc != 4) {
    printf("Usage: program2.c <file1> <file2> <output_file>\n");
    exit(EXIT_FAILURE);
  }

  int fd_prog1[2], fd_prog2[2];
  pipe(fd_prog1);
  pipe(fd_prog2);

  // executing program1 twice
  char *file_name1 = argv[1];
  char *file_name2 = argv[2];
  execute_program(file_name1, fd_prog1[1]);
  execute_program(file_name2, fd_prog2[1]);
  close(fd_prog1[1]);
  close(fd_prog2[1]);
  
  char *output_file = argv[3];
  int output_file_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  size_t read_from_file1, read_from_file2;
  char buffer1[1024], buffer2[1024];

  // reading the programs output and making bitwise XOR
  while ((read_from_file1 = read(fd_prog1[0], buffer1, sizeof(buffer1))) > 0 &&
         (read_from_file2 = read(fd_prog2[0], buffer2, sizeof(buffer2)) > 0)) {
    for (size_t i = 0; i < read_from_file1 && read_from_file2; i++) {
      buffer1[i] ^= buffer2[i];
    }
    write(output_file_fd, buffer1, sizeof(buffer1));
  }
  
  close(fd_prog1[0]);
  close(fd_prog2[0]);
  
  return EXIT_SUCCESS;
}
