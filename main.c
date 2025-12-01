#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "request_line.c"
#include "response.c"

int main() {
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1) {
    printf("socket not initialised \n");
    close(server_socket);
    return 1;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(8080); // Port 8080
  address.sin_addr.s_addr = INADDR_ANY;
  
  int opt = 1;
  if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    printf("set socket failed \n");
    close(server_socket);
    return 1;
  }

  // Bind server_socket to address
  if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0){
    printf("binding failed \n");
    close(server_socket);
    return 1;
  }

  // Allow 4 client with queueing
  if (listen(server_socket, 4) < 0){
    printf("listening failed \n");
    close(server_socket);
    return 1;
  }

  printf("Listening on 8080 \n");

  while (1) {
    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket < 0) {
      printf("client socket failed \n");
      close(client_socket);
      close(server_socket);
      return 1;
    }

    char message[4096];
    memset(message, 0, sizeof(message));
    ssize_t received = 0;
    char *sep = "\r\n\r\n";

    request_t request = {
      .bytes = 0,
      .header_done = false,
      .headers_done = false,
      .headers = hash_init(),
      .state = REQUEST_LINE
    };

    while (received < sizeof(message)) {
      ssize_t bytes = recv(client_socket, message + received, 8, 0);

      if (bytes < 0) {
        printf("receiving message failed at received: %zd \n", received);
        close(client_socket);
        close(server_socket);
        return 1;
      }

      if (bytes == 0) {
        break;
      }

      int content_len_idx = hash_get_node_index("Content-Length", &request.headers);

      switch(request.state) {
        case REQUEST_LINE: 
          if (strstr(message, "\r\n") != NULL ) { // && request.header_done == false) {
            request_from_reader(message, &request);
            // request.header_done = true;
            request.state = HEADERS;
          } 
        case HEADERS: 
          if (strstr(message, sep)) { 
            parse_headers(message, &request);
            content_len_idx = hash_get_node_index("Content-Length", &request.headers);

            if (content_len_idx >= 0) {
              request.state = BODY;
            } else {
              request.state = DONE;
              break;
            }
          }
        case BODY: 
          if (strstr(message, sep)) {
            char *sep_message = strstr(message, sep);

            if (sep_message) {
              int len = strlen(message);
              char *rest = sep_message + strlen(sep);
              parse_body(rest, &request);
            }
            int content_len = atoi(request.headers.nodes[content_len_idx]->value);
            if (content_len == strlen(request.body)) {
              request.state = DONE;
            }
          }
        case DONE: 
          break;
      }

      if (request.state == DONE) {
        response_t response = { .headers = hash_init() };
        write_response_body(&response, &request);
        char *body = response_body(&response);

        send(client_socket, body, strlen(body), 0);
        free(body);
        break;
      }

      received += bytes;
    }

    if (request.error) {
      printf("ERROR %s\n", request.error);
      close(client_socket);
      break;
    }

    printf("Request line:\n");
    printf("- Method: %s\n", request.request_line.method);
    printf("- Target: %s\n", request.request_line.target);
    printf("- Version: %s\n", request.request_line.version);
    printf("Headers :\n");
    for (int i = 0; i < request.headers.size; i++) {
      printf("- %s: %s\n", request.headers.nodes[i]->key, request.headers.nodes[i]->value);
    }
    if (request.body) {
      printf("BODY: %s\n", request.body);
    }

    // printf("CLIENT MESSAGE: %s\n", message);
    request.state = REQUEST_LINE;
    close(client_socket);
  }

  close(server_socket);
  return 0;
}
