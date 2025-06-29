#include "fo_api.h"
#include "../be_api/be_api_common.h"
#include "../build/be_api/colony.pb.h"
#include "../shared/utils.h"
#include "../shared/image_saver.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>

using distributedcolony::PingRequest;
using distributedcolony::PingResponse;
using distributedcolony::InitColonyRequest;
using distributedcolony::InitColonyResponse;
using distributedcolony::GetImageRequest;
using distributedcolony::GetImageResponse;
using distributedcolony::BlastRequest;
using distributedcolony::BlastResponse;
using distributedcolony::Color;

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

void ping_backend(int sock) {
    PingRequest request;
    request.set_client_id("frontend-1");
    std::string out;
    request.SerializeToString(&out);
    const uint32_t func_code = htonl(static_cast<uint32_t>(BackendAPIFunctionCode::PING));
    const uint32_t msg_len = htonl(static_cast<uint32_t>(out.size()));
    send(sock, &func_code, sizeof(func_code), 0);
    send(sock, &msg_len, sizeof(msg_len), 0);
    send(sock, out.data(), out.size(), 0);
    PingResponse response = receive_ping_response(sock);
    print("PING Response = " + std::to_string(response.status()));
}

void init_colony(int sock, int width, int height) {
    InitColonyRequest request;
    request.set_width(width);
    request.set_height(height);
    std::string out;
    request.SerializeToString(&out);
    const uint32_t func_code = htonl(static_cast<uint32_t>(BackendAPIFunctionCode::INIT_COLONY));
    const uint32_t msg_len = htonl(static_cast<uint32_t>(out.size()));
    send(sock, &func_code, sizeof(func_code), 0);
    send(sock, &msg_len, sizeof(msg_len), 0);
    send(sock, out.data(), out.size(), 0);
    // Receive response
    uint32_t resp_func_code, resp_msg_len;
    if (!read_func_code_and_length(sock, resp_func_code, resp_msg_len)) {
        throw std::runtime_error("Failed to read InitColony response function code or length");
    }
    if (resp_func_code != static_cast<uint32_t>(BackendAPIFunctionCode::INIT_COLONY)) {
        throw std::runtime_error("Unexpected function code in InitColony response");
    }
    std::vector<char> buffer(resp_msg_len);
    if (read_n_bytes_from_socket(sock, buffer.data(), resp_msg_len) != static_cast<ssize_t>(resp_msg_len)) {
        throw std::runtime_error("Failed to read InitColony response message");
    }
    InitColonyResponse response;
    if (!response.ParseFromArray(buffer.data(), resp_msg_len)) {
        throw std::runtime_error("Failed to parse InitColonyResponse");
    }
    print("InitColony response status = " + std::to_string(response.status()));
}

void blast(int sock, int x, int y, int radius, Color color) {
    BlastRequest request;
    request.set_x(x);
    request.set_y(y);
    request.set_radius(radius);
    *request.mutable_color() = color;
    std::string out;
    request.SerializeToString(&out);
    const uint32_t func_code = htonl(static_cast<uint32_t>(BackendAPIFunctionCode::BLAST));
    const uint32_t msg_len = htonl(static_cast<uint32_t>(out.size()));
    send(sock, &func_code, sizeof(func_code), 0);
    send(sock, &msg_len, sizeof(msg_len), 0);
    send(sock, out.data(), out.size(), 0);
    // Receive response
    uint32_t resp_func_code, resp_msg_len;
    if (!read_func_code_and_length(sock, resp_func_code, resp_msg_len)) {
        throw std::runtime_error("Failed to read Blast response function code or length");
    }
    if (resp_func_code != static_cast<uint32_t>(BackendAPIFunctionCode::BLAST)) {
        throw std::runtime_error("Unexpected function code in Blast response");
    }
    std::vector<char> buffer(resp_msg_len);
    if (read_n_bytes_from_socket(sock, buffer.data(), resp_msg_len) != static_cast<ssize_t>(resp_msg_len)) {
        throw std::runtime_error("Failed to read Blast response message");
    }
    BlastResponse response;
    if (!response.ParseFromArray(buffer.data(), resp_msg_len)) {
        throw std::runtime_error("Failed to parse BlastResponse");
    }
    print("Blast response status = " + std::to_string(response.status()));
}

void blast(int sock, int x, int y, int radius) {
    blast(sock, x, y, radius, Random::random_color());
}

void get_image_and_save(int sock, int offsetX, int offsetY, int width, int height, const std::string& filename, bool quiet) {
    GetImageRequest request;
    request.set_offsetx(offsetX);
    request.set_offsety(offsetY);
    request.set_width(width);
    request.set_height(height);
    std::string out;
    request.SerializeToString(&out);
    const uint32_t func_code = htonl(static_cast<uint32_t>(BackendAPIFunctionCode::GET_IMAGE));
    const uint32_t msg_len = htonl(static_cast<uint32_t>(out.size()));
    send(sock, &func_code, sizeof(func_code), 0);
    send(sock, &msg_len, sizeof(msg_len), 0);
    send(sock, out.data(), out.size(), 0);
    // Receive response
    uint32_t resp_func_code, resp_msg_len;
    if (!read_func_code_and_length(sock, resp_func_code, resp_msg_len)) {
        throw std::runtime_error("Failed to read GetImage response function code or length");
    }
    if (resp_func_code != static_cast<uint32_t>(BackendAPIFunctionCode::GET_IMAGE)) {
        throw std::runtime_error("Unexpected function code in GetImage response");
    }
    std::vector<char> buffer(resp_msg_len);
    if (read_n_bytes_from_socket(sock, buffer.data(), resp_msg_len) != static_cast<ssize_t>(resp_msg_len)) {
        throw std::runtime_error("Failed to read GetImage response message");
    }
    GetImageResponse response;
    if (!response.ParseFromArray(buffer.data(), resp_msg_len)) {
        throw std::runtime_error("Failed to parse GetImageResponse");
    }
    if (response.status() == 0) {
        std::vector<uint8_t> rgb_data(response.rgbbytes().begin(), response.rgbbytes().end());
        if (ImageSaver::save_png(filename, response.width(), response.height(), rgb_data)) {
            if (!quiet) print("Saved image as " + filename);
        } else {
            if (!quiet) print("Failed to save PNG image: " + filename);
        }
    }
} 