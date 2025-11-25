#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct RequestLine {
  char* method;
  char* target;
  char* version;
  char* error;
} request_line_t;

char * delete_chars(char *initial_string, char *pattern) {
  char *string_copy = strdup(initial_string);
  int pattern_len = strlen(pattern);

  memmove(string_copy, string_copy + pattern_len, pattern_len + 1);
  return string_copy;
}

void validate(request_line_t *request_line) {
  for (int i = 0; i < strlen(request_line->method); i++) {
    if (!isupper(request_line->method[i])) {
      request_line->error = "Method invalid";
      return;
    }
  }

  if (strstr(request_line->version, "HTTP") == NULL) {
    request_line->error = "Request needs to be HTTP";
    return;
  }

  request_line->version = delete_chars(request_line->version, "HTTP/");
  if (strstr(request_line->version, "1.1") == NULL) {
    request_line->error = "Request version needs to be HTTP 1.1";
    return;
  }
} 

request_line_t request_from_reader(char *message) {
  char *first_line = strtok(message, "\r\n");
  request_line_t request_line;
  char **items = malloc(3 * sizeof(char *));

  char *token = strtok(first_line, " ");
  int i = 0;
  while (token != NULL) {
    if (i > 2) {
      request_line.error = "Too many items in the Request Line";
      free(items);
      return request_line;
    }
    items[i] = strdup(token);
    i++;
    token = strtok(NULL, " ");
  }

  if (i < 3) {
    request_line.error = "Incomplete attributes";
    free(items);
    return request_line;
  }

  request_line.method = items[0];
  request_line.target = items[1];
  request_line.version = items[2];
  free(items);

  validate(&request_line);

  return request_line;
}

