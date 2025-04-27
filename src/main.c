#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#ifdef _WIN32
#include <direct.h>
#define GETCWD _getcwd
#else
#include <unistd.h>
#define GETCWD getcwd
#endif

char cwd[512] = {};
char full_path[1024] = {};
char file_name[128] = {};


bool get_current_working_directory(void) {
  return GETCWD(cwd, sizeof(cwd)) != NULL;
}

void generate_header_file(FILE* read_file) {
  char header_file_name[128] = {};
  char header_full_path[1024] = {};

  // Create header filename: file_name + ".h"
  if (strlen(file_name) + 3 >= sizeof(header_file_name)) { // +3 for ".h" and null terminator
      fprintf(stderr, "[ERROR]: Header filename '%s.h' is too long\n", file_name);
      return;
  }
  snprintf(header_file_name, sizeof(header_file_name), "%s.h", file_name);

  // Construct full path for header file in current working directory
  #ifdef _WIN32
      snprintf(header_full_path, sizeof(header_full_path), "%s\\%s", cwd, header_file_name);
  #else
      snprintf(header_full_path, sizeof(header_full_path), "%s/%s", cwd, header_file_name);
  #endif

  // Open the header file for writing
  FILE* header_file = fopen(header_full_path, "w");
  if (header_file == NULL) {
      fprintf(stderr, "[ERROR]: Failed to create header file '%s': %s\n", header_full_path, strerror(errno));
      return;
  }

  // Create include guard name from header filename (uppercase, replace non-alphanumeric with '_')
  char guard_name[256] = {};
  size_t guard_len = 0;
  for (size_t i = 0; header_file_name[i] && guard_len < sizeof(guard_name) - 1; i++) {
      if (isalnum(header_file_name[i])) {
          guard_name[guard_len++] = toupper(header_file_name[i]);
      } else {
          guard_name[guard_len++] = '_';
      }
  }
  guard_name[guard_len] = '\0';

  // Create variable name from filename (replace non-alphanumeric with '_')
  char variable_name[256] = {};
  size_t variable_len = 0;
  for (size_t i = 0; header_file_name[i] && variable_len < sizeof(variable_name) - 1; i++) {
      if (isalnum(header_file_name[i])) {
        variable_name[variable_len++] = header_file_name[i];
      } else {
        variable_name[variable_len++] = '_';
      }
  }
  variable_name[variable_len] = '\0';

  // Write header file content with include guards
  fprintf(header_file, "#ifndef %s\n", guard_name);
  fprintf(header_file, "#define %s\n\n", guard_name);
  
  fprintf(header_file, "char %s[] = {", variable_name);

  fseek(read_file, 0, SEEK_END);
  size_t file_size = ftell(read_file);
  fseek(read_file, 0, SEEK_SET);
  const int column_count = 12;
  uint8_t byte;
  for (int i = 0; i < file_size; i++){
    if (i % column_count == 0){
      fprintf(header_file, "\n\t");
    }

    fread(&byte, sizeof(uint8_t), 1, read_file);
    fprintf(header_file, "0x%02x", byte & 0xff);

    if (i < file_size - 1){
      fprintf(header_file, ", ");
    }
  }
  
  
  fprintf(header_file, "\n};\n");
  fprintf(header_file, "#endif /* %s */\n", guard_name);

  // Close the header file
  fclose(header_file);
  printf("Generated header file: %s\n", header_full_path);
}


int main(int argc, char* argv[]){
  printf("\n");
  if (argc != 2){
    // Expected a relative path to a file
    printf("[ERROR]: No path given\n");
    return 0;
  }
  
  if (!get_current_working_directory()) {
      fprintf(stderr, "[ERROR]: Failed to get current working directory\n");
      return 1;
  }

  char* relative_path = argv[1];

    // Use appropriate path separator for the platform
#ifdef _WIN32
    snprintf(full_path, sizeof(full_path), "%s\\%s", cwd, relative_path);
#else
    snprintf(full_path, sizeof(full_path), "%s/%s", cwd, relative_path);
#endif

    // Open the file
    FILE* file_to_read = fopen(full_path, "rb");
    if (file_to_read == NULL) {
        fprintf(stderr, "[ERROR]: Failed to open file '%s': %s\n", full_path, strerror(errno));
        return 1;
    }

    // Extract filename from relative_path
    const char* last_separator = NULL;
#ifdef _WIN32
    // Check for both \ and / on Windows, as either might be used
    const char* last_backslash = strrchr(relative_path, '\\');
    const char* last_slash = strrchr(relative_path, '/');
    if (last_backslash && last_slash) {
        last_separator = (last_backslash > last_slash) ? last_backslash : last_slash;
    } else {
        last_separator = last_backslash ? last_backslash : last_slash;
    }
#else
    last_separator = strrchr(relative_path, '/');
#endif

    // If no separator, the relative_path is the filename; otherwise, take the part after the separator
    const char* extracted_name = last_separator ? last_separator + 1 : relative_path;

    // Check if filename fits in file_name buffer
    if (strlen(extracted_name) >= sizeof(file_name)) {
        fprintf(stderr, "[ERROR]: Filename '%s' is too long\n", extracted_name);
        fclose(file_to_read);
        return 1;
    }

    // Copy filename to file_name
    strncpy(file_name, extracted_name, sizeof(file_name) - 1);
    file_name[sizeof(file_name) - 1] = '\0'; // Ensure null-termination

    generate_header_file(file_to_read);

    // Cleanup
    fclose(file_to_read);

  return 0;
}