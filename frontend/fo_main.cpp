#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../be_api/be_api_common.h"
#include "../build/be_api/ping.pb.h"
#include <vector>
#include <stdexcept>

using distributedcolony::PingRequest;
using distributedcolony::PingResponse;

void print(const std::string& msg) {
    std::cout << "[FO] " << msg << std::endl;
}

int create_and_connect_socket(const char* ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        print("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        print("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        print("Connection failed");
        return -1;
    }

    print("Connection established");
    return sock;
}

PingResponse receive_ping_response(int sock) {
    uint32_t func_code, msg_len;
    if (!read_func_code_and_length(sock, func_code, msg_len)) {
        throw std::runtime_error("Failed to read response function code or length");
    }
    if (func_code != static_cast<uint32_t>(BackendAPIFunctionCode::PING)) {
        throw std::runtime_error("Unexpected function code in response");
    }
    std::vector<char> buffer(msg_len);
    if (read_n_bytes_from_socket(sock, buffer.data(), msg_len) != static_cast<ssize_t>(msg_len)) {
        throw std::runtime_error("Failed to read response message");
    }

    PingResponse response;
    if (!response.ParseFromArray(buffer.data(), msg_len)) {
        throw std::runtime_error("Failed to parse PingResponse");
    }
    return response;
}

PingResponse ping_backend(int sock) {
    PingRequest request;
    request.set_client_id("frontend-1");

    std::string out;
    request.SerializeToString(&out);

    const uint32_t func_code = htonl(static_cast<uint32_t>(BackendAPIFunctionCode::PING));
    const uint32_t msg_len = htonl(static_cast<uint32_t>(out.size()));
    send(sock, &func_code, sizeof(func_code), 0);
    send(sock, &msg_len, sizeof(msg_len), 0);
    send(sock, out.data(), out.size(), 0);
    return receive_ping_response(sock);
}

int main() {
    const int sock = create_and_connect_socket("127.0.0.1", BE_API_PORT);
    if (sock < 0) {
        return 1;
    }
    PingResponse response = ping_backend(sock);
    print("PING Response = " + std::to_string(response.status()));

    print("Closing connection");
    close(sock);
    return 0;
}