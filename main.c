#include <stdio.h>
#include <stdlib.h>
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
    int received = 0;

    while (received < sizeof(message)) {
      int bites = recv(client_socket, message + received, 8, 0);

      if (bites < 0) {
        printf("receiving message failed at received: %d \n", received);
        close(client_socket);
        close(server_socket);
        return 1;
      }

      // client closed connection
      if (bites == 0) {
        break;
      }

      received += bites;
      message[received] = '\0';
    }

    request_line_t request_line = request_from_reader(message);
    printf(
      "REQUST LINE method: %s target: %s version: %s\n",
      request_line.method,
      request_line.target,
      request_line.version
    );
    // printf("CLIENT MESSAGE: %s", message);

    // // Send message to the client
    // if (send(client_socket, message, strlen(message), 0) < 0) {
    //   printf("sending message failed \n");
    //   close(client_socket);
    //   close(server_socket);
    //   return 1;
    // }

    free(request_line.method);
    free(request_line.target);
    free(request_line.version);

    close(client_socket);
  }


  close(server_socket);
  return 0;
}
