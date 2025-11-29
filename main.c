#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "request_line.c"

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

  // Listen for clients and allow the accept function to be used
  // Allow 4 client with queueing
  if (listen(server_socket, 4) < 0){
    printf("listening failed \n");
    close(server_socket);
    return 1;
  }

  printf("Listening on 8080 \n");

  while (1) {
    // Wait for client to connect, then open a socket
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

    request_t request = {
      .bytes = 0,
      .header_done = false,
      .headers = hash_init()
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

      char *str_end = strstr(message, "\r\n");
      if (str_end) {
        int end_index = message - str_end;
        message[end_index] = '\0';
      }

      if (strstr(message, "\r\n") != NULL && request.header_done == false) {
        request_from_reader(message, &request);
        request.header_done = true;
      } 

      received += bytes;
      message[received] = '\0';

      if (strstr(message, "\r\n\r\n")) {
        parse_headers(message, &request);
        break;
      }
    }

    if (request.error ) {
      printf("ERROR %s\n", request.error);
      close(client_socket);
      break;
    } else if (request.request_line.error){
      printf("ERROR %s\n", request.request_line.error);
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

    // printf("CLIENT MESSAGE: %s", message);
    printf("DONE WITH THE REQUEST \n");


    // Need to read 8 bytes at a time until the end of the line
    // Format request line
    // Read 8 bytes at a time until you get to the end of the headers
    // Format the headers
    // Check the go solution

    // // Send message to the client
    // if (send(client_socket, message, strlen(message), 0) < 0) {
    //   printf("sending message failed \n");
    //   close(client_socket);
    //   close(server_socket);
    //   return 1;
    // }

    // free(request_line.method);
    // free(request_line.target);
    // free(request_line.version);

    close(client_socket);
  }


  close(server_socket);
  return 0;
}
