#ifndef BE_API_COMMON_H
#define BE_API_COMMON_H

#include <cstdint>
#include <cstddef>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>

constexpr int BE_API_PORT = 8082;

// Shared function codes for API
enum class BackendAPIFunctionCode : uint32_t {
    PING = 1,
    INIT_COLONY = 2,
    GET_IMAGE = 3,
    BLAST = 4
};

inline ssize_t read_n_bytes_from_socket(int fd, void* buf, size_t n) {
    size_t total = 0;
    char* ptr = static_cast<char*>(buf);
    while (total < n) {
        ssize_t bytes = ::read(fd, ptr + total, n - total);
        if (bytes <= 0) return -1;
        total += bytes;
    }
    return total;
}

inline bool write_func_code_and_length(int sock, uint32_t func_code, uint32_t msg_len) {
    func_code = htonl(func_code);
    msg_len = htonl(msg_len);
    return send(sock, &func_code, sizeof(func_code), 0) == sizeof(func_code) &&
           send(sock, &msg_len, sizeof(msg_len), 0) == sizeof(msg_len);
}

inline bool read_func_code_and_length(int sock, uint32_t& func_code, uint32_t& msg_len) {
    uint32_t func_code_net, msg_len_net;
    if (read_n_bytes_from_socket(sock, &func_code_net, sizeof(func_code_net)) != sizeof(func_code_net)) return false;
    if (read_n_bytes_from_socket(sock, &msg_len_net, sizeof(msg_len_net)) != sizeof(msg_len_net)) return false;
    func_code = ntohl(func_code_net);
    msg_len = ntohl(msg_len_net);
    return true;
}

#endif // BE_API_COMMON_H 