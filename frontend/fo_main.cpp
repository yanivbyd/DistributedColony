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
#include "../shared/utils.h"
#include "../shared/image_saver.h"
#include <string>

const int COLONY_WIDTH = 500;
const int COLONY_HEIGHT = 500;

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
        // Convert string to vector<uint8_t>
        std::vector<uint8_t> rgb_data(response.rgbbytes().begin(), response.rgbbytes().end());
        
        if (ImageSaver::save_png("output/colony.png", response.width(), response.height(), rgb_data)) {
            print("Saved image as output/colony.png");
        } else {
            print("Failed to save PNG image");
        }
    }
}

void blast(int sock, int x, int y, int radius) {
    BlastRequest request;
    request.set_x(x);
    request.set_y(y);
    request.set_radius(radius);
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

struct CommandLineArgs {
    bool should_init_colony = true;
    bool should_exit = false;
    int exit_code = 0;
};

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --init-colony [true|false]  Initialize a new colony (default: true)" << std::endl;
    std::cout << "  --help                      Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                    # Initialize new colony" << std::endl;
    std::cout << "  " << program_name << " --init-colony true  # Initialize new colony" << std::endl;
    std::cout << "  " << program_name << " --init-colony false # Skip initialization" << std::endl;
}

CommandLineArgs parse_arguments(int argc, char* argv[]) {
    CommandLineArgs args;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--init-colony") {
            // Check if next argument is a value
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                std::string value = argv[i + 1];
                if (value == "true" || value == "1") {
                    args.should_init_colony = true;
                } else if (value == "false" || value == "0") {
                    args.should_init_colony = false;
                } else {
                    std::cout << "Invalid value for --init-colony: " << value << std::endl;
                    print_usage(argv[0]);
                    args.should_exit = true;
                    args.exit_code = 1;
                }
                i++; // Skip the value argument
            } else {
                // No value provided, default to true
                args.should_init_colony = true;
            }
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            args.should_exit = true;
            args.exit_code = 0;
        } else {
            std::cout << "Unknown argument: " << arg << std::endl;
            print_usage(argv[0]);
            args.should_exit = true;
            args.exit_code = 1;
        }
    }
    
    return args;
}

int main(int argc, char* argv[]) {
    CommandLineArgs args = parse_arguments(argc, argv);
    
    if (args.should_exit) {
        return args.exit_code;
    }
    
    const int sock = create_and_connect_socket("127.0.0.1", BE_API_PORT);
    if (sock < 0) {
        return 1;
    }
    
    ping_backend(sock);
    
    if (args.should_init_colony) {
        print("Initializing new colony with dimensions " + std::to_string(COLONY_WIDTH) + "x" + std::to_string(COLONY_HEIGHT));
        init_colony(sock, COLONY_WIDTH, COLONY_HEIGHT);
        blast(sock, Random::range(0, 200), Random::range(0, 200), Random::range(3, 60));
    } else {
        print("Skipping colony initialization (use --init-colony to create a new colony)");
    }
    
    get_image(sock, 0, 0, COLONY_WIDTH, COLONY_HEIGHT);
    print("Closing connection");
    close(sock);
    return 0;
}