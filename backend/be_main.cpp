#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../be_api/be_api_common.h"

int create_listening_socket(int port) {
    int server_fd;
    struct sockaddr_in address;
    const int opt = 1;
    const int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEADDR");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void accept_and_handle_connections(int server_fd) {
    int new_socket;
    struct sockaddr_in address;
    const int addrlen = sizeof(address);
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
        char buffer[1024];
        while (read(server_fd, buffer, sizeof(buffer)) > 0) {
            // Discard the data (for now)
        }
        close(new_socket);
    }
}

int main() {
    int server_fd = create_listening_socket(BE_API_PORT);
    std::cout << "Backend listening on port " << BE_API_PORT << std::endl;
    accept_and_handle_connections(server_fd);
    close(server_fd);
    return 0;
} 