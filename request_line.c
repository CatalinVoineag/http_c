#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hash.c"

typedef struct RequestLine {
  char* method;
  char* target;
  char version[4];
  char* error;
} request_line_t;

typedef struct Headers {
  char* host;
  char* error;
} headers_t;

typedef struct Request {
  char header[4096];
  int bytes;
  char* error;
  request_line_t request_line;
  hash_t headers;
  bool header_done;
} request_t;

char* downcase(char* string) {
  for (int i = 0; i < strlen(string); i++) {
    string[i] = tolower(string[i]);
  }

  return string;
}

void set_version(request_line_t *request_line, char *string) {
  char *version = strstr(string, "1.1");
  if (version) {
    request_line->version[0] = version[0]; // 1
    request_line->version[1] = version[1]; // .
    request_line->version[2] = version[2]; // 1
  }
  printf("SET VERSION %s\n", request_line->version);
}

char * delete_chars(char *initial_string, char *pattern) {
  char *string_copy = strdup(initial_string);
  int pattern_len = strlen(pattern);

  memmove(string_copy, string_copy + pattern_len, pattern_len + 1);
  return string_copy;
}

void del_chars(char *initial_string, char *pattern) {
  int pattern_len = strlen(pattern);

  memmove(initial_string, initial_string + pattern_len, pattern_len + 1);
}

void validate(request_line_t *request_line) {
  for (int i = 0; i < strlen(request_line->method); i++) {
    if (!isupper(request_line->method[i])) {
      request_line->error = "Method invalid";
      return;
    }
  }

  if (strcmp(request_line->version, "1.1") != 0) {
    request_line->error = "Request version needs to be HTTP 1.1";
    return;
  }
} 

void request_from_reader(char *line, request_t *request) {
  request_line_t request_line;
  char **items = malloc(3 * sizeof(char *));

  char *token = strtok(strdup(line), " ");
  int i = 0;
  while (token != NULL) {
    if (i > 2) {
      request->error = "Too many items in the Request Line";
      free(items);
      return;
    }
    items[i] = strdup(token);
    i++;
    token = strtok(NULL, " ");
  }

  if (i < 3) {
    request->error = "Incomplete attributes";
    free(items);
    return;
  }

  request_line.method = items[0];
  request_line.target = items[1];
  set_version(&request_line, items[2]);
  free(items);

  validate(&request_line);
  request->request_line = request_line;

  return;
}

void partition(char *string, char *delimiter, hash_t *hash) {
  char *arr[2];

  for (int i = 0; i < strlen(string); i++) {
    if (memcmp(string + i, ":", 1) == 0) {
      char left[i];
      arr[0] = memcpy(left, string, i);
      arr[0][i] = '\0';

      int right_len = strlen(string + (i+1));
      char right[right_len];
      arr[1] = memcpy(right, string + (i +1), right_len);
      arr[1][right_len] = '\0';

      char *key = arr[0]; 
      char *value = arr[1]; 

      int node_index = hash_get_node_index(key, hash);
      if (node_index >= 0) {
        hash_append(key, value, hash); 
      } else {
        hash_add(key, value, hash); 
      }

      break;
    }
  }
} 

void parse_headers(char *message, request_t *request) {
  char *token = strtok(message, "\r\n");

  while (token != NULL) {
    partition(token, ":", &request->headers);
    token = strtok(NULL, "\r\n");
  }
}
