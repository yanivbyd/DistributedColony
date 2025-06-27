#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../be_api/be_api_common.h"

int create_and_connect_socket(const char* ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "[FO] Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        std::cerr << "[FO] Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "[FO] Connection Failed" << std::endl;
        return -1;
    }

    std::cout << "[FO] Connection established" << std::endl;
    return sock;
}

int main() {
    int sock = create_and_connect_socket("127.0.0.1", BE_API_PORT);
    if (sock < 0) {
        return 1; // Connection failed, exit with error
    }
    std::cout << "[FO] Closing connection" << std::endl;
    close(sock);
    return 0;
}