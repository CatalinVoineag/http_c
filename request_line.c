#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "hash.c"

#ifndef REQUEST_LINE_H
#define REQUEST_LINE_H

typedef struct RequestLine {
  char* method;
  char* target;
  char version[4];
} request_line_t;

typedef enum { REQUEST_LINE, HEADERS, BODY, DONE } state_t;

typedef struct Request {
  char header[4096];
  int bytes;
  char* error;
  request_line_t request_line;
  hash_t headers;
  bool header_done;
  bool headers_done;
  state_t state;
  char* body;
} request_t;

void trim(char *string) {
  int i = 0;
  int j = 0;

  while (string[i] == ' ') i++;

  while ((string[j++] = string[i++]));
}

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
    request_line->version[3] = '\0';
  }
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

void validate(request_line_t *request_line, request_t *request) {
  for (int i = 0; i < strlen(request_line->method); i++) {
    if (!isupper(request_line->method[i])) {
      request->error = "Method invalid";
      return;
    }
  }

  if (strcmp(request_line->version, "1.1") != 0) {
    request->error = "Request version needs to be HTTP 1.1";
    return;
  }
} 

void request_from_reader(char *line, request_t *request) {
  request_line_t request_line;
  char **items = malloc(3 * sizeof(char *));

  char *token = strtok(strdup(line), " ");
  int i = 0;
  while (token != NULL) {
    if (i > 2) { break; }

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

  validate(&request_line, request);
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
      trim(value);

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
  char *token = strtok(strdup(message), "\r\n");

  while (token != NULL) {
    partition(token, ":", &request->headers);
    token = strtok(NULL, "\r\n");
  }
}

void parse_body(char *message, request_t *request) {
  if (request->body == NULL) {
    request->body = strdup(message);
  } else {
    int body_len = strlen(request->body);
    char *mess = strdup(message);
    int remaining = strlen(mess) - body_len;

    if (remaining < 0) remaining = 0;

    memcpy(request->body + body_len, mess + body_len, remaining);
    request->body[body_len + remaining] = '\0';
  }
}

#endif
