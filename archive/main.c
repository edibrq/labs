#include <stdio.h>
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>

#define FILE_START "<FILE_START>"
#define FILE_END  "<FILE_END>"

#define HEADER_START "<HEADER_START>"
#define HEADER_END "<HEADER_END>"

#define PASSWORD_START "<PASSWORD_START>"
#define PASSWORD_END "<PASSWORD_END>"

typedef struct file_info {
  char *name;
  char *content;
  long size;
  struct file_info* next;
} file_info;

void print_help() {
  printf("usage: <dir_name> <out_file>\n");
}

file_info* read_file_content(char* file_name) {
  char* buffer = NULL;
  long file_size;
  FILE* file = fopen(file_name, "rb");

  if (!file) {
    printf("Can't open file: %s\n", file_name);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  
  buffer = (char *) malloc(file_size);
  if (!buffer) {
    printf("No buffer for file_name: %s\n", file_name);
    return NULL;
  }
  fread(buffer, 1, file_size, file);
  
  file_info *current_file_info = (struct file_info*) malloc(sizeof(file_info));
  char* name = (char*) malloc(strlen(file_name));
  current_file_info->name = strcpy(name, file_name); current_file_info->size = file_size; current_file_info->content = buffer; current_file_info->next = NULL;
  
  return current_file_info;
}

void print_file_info(file_info* file_info_list) {
  file_info* head = file_info_list;
  while (head) {
    printf("File size: %ld\n", head->size);
    printf("File name: %s\n", head->name);
    printf("File content:%s\n\n", head->content);
    head = head->next;
  }
}

file_info* get_dir_content(char * path, file_info* file_info_list) {
  DIR * d = opendir(path); // open the path
  if (d == NULL)
    return file_info_list;
  
  struct dirent * dir; // for the directory entries
  while ((dir = readdir(d)) != NULL) {
      if (dir-> d_type != DT_DIR) {
        char file_path[255];
        //printf("%s%s\n", BLUE, dir->d_name); // blue is a color of files
        snprintf(file_path, 255, "%s/%s", path, dir->d_name);
        file_info* current_file_info = read_file_content(file_path);
        if (!current_file_info) {
          printf("No file info!\n");
          return file_info_list;
        }
        file_info_list->next = current_file_info;
        file_info_list = current_file_info;
      }
      else if(dir -> d_type == DT_DIR && strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name,"..") != 0 ) {
        //printf("directory: %s%s\n", GREEN, dir->d_name); // print its name in green
        char next_dir[255];
        snprintf(next_dir, 255, "%s/%s", path, dir->d_name);
        file_info_list = get_dir_content(next_dir, file_info_list); 
      }
    }
    closedir(d); 
    return file_info_list;
}

void dump_to_file(file_info *file_info_list, char *out_file_name, char *password) {
  FILE *output_file = fopen(out_file_name, "w");
  if (!output_file) {
    printf("Can't create output file: %s", out_file_name);
  }

  fprintf(output_file, "%s\n", PASSWORD_START);
  fprintf(output_file, "%s\n", password);
  fprintf(output_file, "%s\n", PASSWORD_END);

  file_info *head = file_info_list;
  fprintf(output_file, "%s\n", HEADER_START);
  while (head) {
    fprintf(output_file, "%s\n", head->name);
    fprintf(output_file, "%ld\n", head->size);
    head = head->next;
  }
  fprintf(output_file, "%s\n", HEADER_END);

  head = file_info_list;
  while (head) {
    fprintf(output_file, "%s", head->content);
    head = head->next;
  }
  fclose(output_file);
}

char* read_line(FILE *file) {
    int max_line_len = 128;

    char *line_buffer = (char *)malloc(sizeof(char) * max_line_len);
    if (line_buffer == NULL) {
        printf("Error allocating memory for line buffer.");
        exit(1);
    }

    char ch = getc(file);
    int count = 0;
    while ((ch != '\n') && (ch != EOF)) {
        if (count == max_line_len) {
            max_line_len += 128;
            line_buffer = (char*) realloc(line_buffer, max_line_len);
            if (line_buffer == NULL) {
                printf("Error reallocating space for line buffer.");
                exit(1);
            }
        }
        line_buffer[count] = ch;
        count++;

        ch = getc(file);
    }

    line_buffer[count] = '\0';
    return line_buffer;
}

void parse_input(char *input_file_path, file_info *file_info_list, char *password) {
  FILE *file = fopen(input_file_path, "r");
  
  char *file_line = read_line(file);
  char *archived_password = read_line(file);
  if (strcmp(password, archived_password)) {
    printf("Incorrect password, exiting...\n");
    exit(1);
  }
  file_line = read_line(file);
  // read header section
  read_line(file);
  file_info *head = file_info_list;
  while (strcmp(HEADER_END, file_line)) {
    file_info* current_file_info = (file_info *) malloc(sizeof(file_info));
    
    file_line = read_line(file);
    if (!strcmp(HEADER_END, file_line))
      break;
    current_file_info->name = file_line;
    
    file_line = read_line(file);
    current_file_info->size = atoi(file_line);

    head->next = current_file_info;
    head = head->next;
  }

  // read files section
  head = file_info_list->next;
  while (head) {
    char *file_content = (char *) malloc(head->size * sizeof(char));
    for (int i = 0; i < head->size; i++)
      file_content[i] = getc(file);
   
    file_content[head->size] = '\0';
    head->content = file_content;
    head = head->next;
  }
  fclose(file);
}

void create_directories(char *dir_name) {
  char *sep = strrchr(dir_name, '/');
  if (sep != NULL) {
    *sep = 0;
    create_directories(dir_name);
    *sep = '/';
  }
  printf("dir_name to create: %s\n", dir_name);
  mkdir(dir_name, 0777);
}

void create_file(file_info* current_file_info) {
  FILE* file = fopen(current_file_info->name, "w");
  fprintf(file, "%s", current_file_info->content);
  fclose(file);
}

void unpack(file_info* file_info_list) {
    file_info* current = file_info_list;
    while (current) {
        char* full_path = current->name;
        char* dir_path = dirname(full_path);
        create_directories(dir_path);
        create_file(current);
        current = current->next;
    }
}

char *request_password() {
  char password[255];

  printf("Enter the password\n");
  scanf("%s", password);
  return strdup(password);
}

int main(int argc, char** argv) {
  if (!strcmp(argv[1], "-d") && !strcmp(argv[3], "-o")) {
    printf("creating archive...\n");
    char *dir_name = argv[2];
    char *out_file = argv[4];
    char *password = request_password();

    file_info *file_info_list = (file_info *) malloc(sizeof(file_info));
    file_info *head = file_info_list;

    get_dir_content(dir_name, file_info_list);
    dump_to_file(head->next, out_file, password);
  } else if (!strcmp(argv[1], "-f")) {
    printf("start parsing...\n");
    char *input_file_path = argv[2];
    char *password = request_password();

    file_info *file_info_list = (file_info *) malloc(sizeof(file_info));
    file_info *head = file_info_list;
    
    parse_input(input_file_path, file_info_list, password);
    unpack(file_info_list->next);
  } else {
    print_help();
  }
}
