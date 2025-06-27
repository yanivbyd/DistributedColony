#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include "../be_api/be_api_common.h"
#include "../build/be_api/colony.pb.h"
#include "be_colony.h"

using enum BackendAPIFunctionCode;
using distributedcolony::InitColonyRequest;
using distributedcolony::InitColonyResponse;

void print(const std::string& msg) {
    std::cout << "[BE] " << msg << std::endl;
}

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

void handle_client_requests(const int client_socket) {
    while (true) {
        uint32_t func_code, msg_len;
        if (!read_func_code_and_length(client_socket, func_code, msg_len)) {
            break;
        }
        const BackendAPIFunctionCode code = static_cast<BackendAPIFunctionCode>(func_code);
        std::vector<char> buffer(msg_len);
        if (read_n_bytes_from_socket(client_socket, buffer.data(), msg_len) != static_cast<ssize_t>(msg_len)) {
            break;
        }
        switch (code) {
            case PING: {
                distributedcolony::PingRequest request;
                if (request.ParseFromArray(buffer.data(), msg_len)) {
                    print("Received PING from client_id: " + request.client_id());
                    // Prepare and send PingResponse
                    distributedcolony::PingResponse response;
                    response.set_status(0);
                    std::string out;
                    response.SerializeToString(&out);
                    write_func_code_and_length(client_socket, static_cast<uint32_t>(code), out.size());
                    send(client_socket, out.data(), out.size(), 0);
                } else {
                    print("Failed to parse PingRequest");
                }
                break;
            }
            case INIT_COLONY: {
                InitColonyRequest request;
                if (request.ParseFromArray(buffer.data(), msg_len)) {
                    print("Received INIT_COLONY with width=" + std::to_string(request.width()) + ", height=" + std::to_string(request.height()));
                    init_colony(request.width(), request.height());
                    InitColonyResponse response;
                    response.set_status(0);
                    std::string out;
                    response.SerializeToString(&out);
                    write_func_code_and_length(client_socket, static_cast<uint32_t>(INIT_COLONY), out.size());
                    send(client_socket, out.data(), out.size(), 0);
                    print("Sent INIT_COLONY response");
                } else {
                    print("Failed to parse InitColonyRequest");
                }
                break;
            }
            default:
                print("Unknown function code: " + std::to_string(func_code));
                break;
        }
    }
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
        handle_client_requests(new_socket);
        close(new_socket);
        print("Connection closed.");
    }
}

int main() {
    int server_fd = create_listening_socket(BE_API_PORT);
    std::cout << "[BE] Backend listening on port " << BE_API_PORT << std::endl;
    accept_and_handle_connections(server_fd);
    close(server_fd);
    return 0;
} 