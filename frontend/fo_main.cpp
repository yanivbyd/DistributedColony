#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "../be_api/be_api_common.h"
#include "../build/be_api/colony.pb.h"
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sys/stat.h>

const int COLONY_WIDTH = 50;
const int COLONY_HEIGHT = 50;

using distributedcolony::PingRequest;
using distributedcolony::PingResponse;
using distributedcolony::InitColonyRequest;
using distributedcolony::InitColonyResponse;
using distributedcolony::GetImageRequest;
using distributedcolony::GetImageResponse;
using distributedcolony::BlastRequest;
using distributedcolony::BlastResponse;

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

void save_bmp(const std::string& filename, int width, int height, const std::string& rgb) {
    // Ensure output directory exists
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        mkdir("output", 0755);
    }
    // Each row must be padded to a multiple of 4 bytes
    int row_padded = (width * 3 + 3) & (~3);
    int filesize = 54 + row_padded * height;
    std::vector<unsigned char> bmpfile(filesize, 0);

    // BMP Header
    bmpfile[0] = 'B'; bmpfile[1] = 'M';
    bmpfile[2] = filesize; bmpfile[3] = filesize >> 8; bmpfile[4] = filesize >> 16; bmpfile[5] = filesize >> 24;
    bmpfile[10] = 54; // Pixel data offset
    bmpfile[14] = 40; // DIB header size
    bmpfile[18] = width; bmpfile[19] = width >> 8; bmpfile[20] = width >> 16; bmpfile[21] = width >> 24;
    bmpfile[22] = height; bmpfile[23] = height >> 8; bmpfile[24] = height >> 16; bmpfile[25] = height >> 24;
    bmpfile[26] = 1; bmpfile[28] = 24; // 1 plane, 24 bits per pixel

    // Write pixel data (BMP is bottom-up, BGR)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int src_idx = ((height - 1 - y) * width + x) * 3;
            int dst_idx = 54 + y * row_padded + x * 3;
            bmpfile[dst_idx + 0] = rgb[src_idx + 2]; // Blue
            bmpfile[dst_idx + 1] = rgb[src_idx + 1]; // Green
            bmpfile[dst_idx + 2] = rgb[src_idx + 0]; // Red
        }
    }
    std::ofstream ofs(filename, std::ios::binary);
    ofs.write(reinterpret_cast<const char*>(bmpfile.data()), bmpfile.size());
}

void get_image(int sock, int offsetX, int offsetY, int width, int height) {
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
    print("GetImage response status = " + std::to_string(response.status()) + ", image size = " + std::to_string(response.rgbbytes().size()));
    if (response.status() == 0) {
        save_bmp("output/colony.bmp", response.width(), response.height(), response.rgbbytes());
        print("Saved image as output/colony.bmp");
    }
}

void blast(int sock, int x, int y) {
    BlastRequest request;
    request.set_x(x);
    request.set_y(y);
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

int main() {
    const int sock = create_and_connect_socket("127.0.0.1", BE_API_PORT);
    if (sock < 0) {
        return 1;
    }
    ping_backend(sock);
    init_colony(sock, COLONY_WIDTH, COLONY_HEIGHT);
    blast(sock, 10, 10);
    get_image(sock, 0, 0, COLONY_WIDTH, COLONY_HEIGHT);
    print("Closing connection");
    close(sock);
    return 0;
}