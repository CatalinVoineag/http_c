#include "hash.c"
#include "request_line.c"
#include <stdio.h>
#include <string.h>

typedef struct Response {
  char* request_line;
  hash_t headers;
} response_t;

// HTTP/1.1 200 OK
void write_response_request_line(response_t *response, request_t *request) {
  if (request->error) {
    response->request_line = "HTTP/1.1 500 BAD REQUEST";
  } else {
    response->request_line = "HTTP/1.1 200 OK";
  }
}

void write_response_body(response_t *response, request_t *request) {
  write_response_request_line(response, request);
  
  hash_add("Content-Length", "0", &response->headers); 
  hash_add("Connection", "close", &response->headers); 
  hash_add("Content-Type", "text/plain", &response->headers); 
}

char *response_body(response_t * response) {
  char *resp = malloc(1024);
  int len = strlen(response->request_line) + 3; // +3 for \r\n and %s
  snprintf(resp, len, "%s\r\n", response->request_line);

  for (int i = 0; i < response->headers.size; i++) {
    char header[1024];
    header[0] = '\0';
    const char *key   = response->headers.nodes[i]->key;
    const char *value = response->headers.nodes[i]->value;

    int message_len = strlen(key) + strlen(value) + 5; // for the \r\n and %s:

    snprintf(
      header,
      message_len,
      "%s: %s\r\n",
      key,
      value
    );

    strcat(resp, header);
  }
  return resp;
}
