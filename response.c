#include "hash.c"
#include "request_line.c"
#include <stdio.h>

#include <string.h>
#include <stdlib.h>


typedef struct Response {
  char* request_line;
  hash_t headers;
  char *body;
} response_t;

typedef enum { YOUR_PROBLEM, MY_PROBLEM, NVIM } special_request_t;

// HTTP/1.1 200 OK
void write_response_request_line(response_t *response, request_t *request, special_request_t special_request) {
  if (request->error) {
    response->request_line = "HTTP/1.1 500 BAD REQUEST";
    return;
  }

  switch (special_request) {
    case YOUR_PROBLEM:
      response->request_line = "HTTP/1.1 500 BAD REQUEST";
      break;
    case MY_PROBLEM:
      response->request_line = "HTTP/1.1 400 INTERNAL SERVER ERROR";
      break;
    default:
      response->request_line = "HTTP/1.1 200 OK";
  }
}

int special_request(request_t *request) {
  if (strcmp(request->request_line.method, "GET") == 0 &&  strcmp(request->request_line.target, "/yourproblem") == 0) {
    return YOUR_PROBLEM;
  } else if (strcmp(request->request_line.method, "GET") == 0 &&  strcmp(request->request_line.target, "/myproblem") == 0) {
    return MY_PROBLEM;
  } else if (strcmp(request->request_line.method, "GET") == 0 &&  strcmp(request->request_line.target, "/use-nvim") == 0) {
    return NVIM;
  } else {
    return -1;
  }
}

char *response_400 = R"(<html>
  <head>
    <title>400 Bad Request</title>
  </head>
  <body>
    <h1>Bad Request</h1>
    <p>Your request honestly kinda sucked.</p>
  </body>
</html>)";

char *response_500 = R"(<html>
  <head>
    <title>500 Internal Server Error</title>
  </head>
  <body>
    <h1>Internal Server Error</h1>
    <p>Okay, you know what? This one is on me.</p>
  </body>
</html>)";

char *response_200 = R"(<html>
  <head>
    <title>200 OK</title>
  </head>
  <body>
    <h1>Success!</h1>
    <p>Your request was an absolute banger.</p>
  </body>
</html>)";

void handle_special_requests(response_t *response, request_t *request, special_request_t special_request) {
  switch(special_request) {
    case YOUR_PROBLEM:
      response->body = response_500;
      break;
    case MY_PROBLEM:
      response->body = response_400;
      break;
    case NVIM:
      response->body = response_200;
      break;
    default:
      hash_add("Content-Length", "0", &response->headers); 
      return;
    }

  char len_str[256];
  len_str[0] = '\0';
  sprintf(len_str, "%lu", strlen(response->body));

  hash_add("Content-Length", len_str, &response->headers); 
}

void write_response_body(response_t *response, request_t *request) {
  special_request_t sp_request = special_request(request);
  handle_special_requests(response, request, sp_request);

  write_response_request_line(response, request, sp_request);
  
  hash_add("Connection", "close", &response->headers); 
  hash_add("Content-Type", "text/html", &response->headers); 
}

char *response_body(response_t * response) {
  char *resp = malloc(1024);
  int len = strlen(response->request_line) + 3; // +3 for \r\n and %s
  snprintf(resp, len, "%s\r\n", response->request_line);

  for (int i = 0; i < response->headers.size; i++) {
    char header[512];
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

  if (response->body) {
    char body[1024];
    body[0] = '\0';

    snprintf(
      body,
      strlen(response->body) + 3,
      "\r\n%s",
      response->body
    );
    strcat(resp, body);
  }

  return resp;
}
