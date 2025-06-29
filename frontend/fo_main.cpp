#include "fo_api.h"
#include "../be_api/be_api_common.h"
#include "../shared/utils.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

const int COLONY_WIDTH = 500;
const int COLONY_HEIGHT = 500;

struct CommandLineArgs {
    bool should_init_colony = true;
    bool should_exit = false;
    int exit_code = 0;
    int video_frame_count = 0; // 0 means no video
};

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --init-colony [true|false]  Initialize a new colony (default: true)" << std::endl;
    std::cout << "  --video <count>             Save a sequence of images and create a video" << std::endl;
    std::cout << "  --help                      Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << program_name << "                    # Initialize new colony" << std::endl;
    std::cout << "  " << program_name << " --init-colony true  # Initialize new colony" << std::endl;
    std::cout << "  " << program_name << " --init-colony false # Skip initialization" << std::endl;
    std::cout << "  " << program_name << " --video 5             # Save 5 images and create a video" << std::endl;
}

CommandLineArgs parse_arguments(int argc, char* argv[]) {
    CommandLineArgs args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--init-colony") {
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
                i++;
            } else {
                args.should_init_colony = true;
            }
        } else if (arg == "--video") {
            if (i + 1 < argc) {
                args.video_frame_count = std::stoi(argv[++i]);
                if (args.video_frame_count <= 0) {
                    std::cout << "Invalid value for --video: must be > 0\n";
                    args.should_exit = true;
                    args.exit_code = 1;
                }
            } else {
                std::cout << "Missing value for --video\n";
                args.should_exit = true;
                args.exit_code = 1;
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
    if (args.video_frame_count > 0) {
        print("Saving video frames...");
        char filename[256];
        int barWidth = 40;
        for (int i = 0; i < args.video_frame_count; ++i) {
            snprintf(filename, sizeof(filename), "output/colony_%04d.png", i+1);
            get_image_and_save(sock, 0, 0, COLONY_WIDTH, COLONY_HEIGHT, filename, true);
            float progress = float(i + 1) / args.video_frame_count;
            std::cout << "\r[";
            int pos = barWidth * progress;
            for (int j = 0; j < barWidth; ++j) {
                if (j < pos) std::cout << "=";
                else if (j == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(progress * 100.0) << "% (" << (i+1) << "/" << args.video_frame_count << ")" << std::flush;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout << std::endl;
        print("Creating video with ffmpeg...");
        int ret = system("ffmpeg -y -framerate 1 -i output/colony_%04d.png -vf \"setpts=PTS/3\" -c:v libx264 -pix_fmt yuv420p output/colony_video.mp4 > /dev/null 2>&1");
        if (ret == 0) {
            print("Video created as output/colony_video.mp4");
        } else {
            print("Failed to create video with ffmpeg");
        }
    } else {
        get_image_and_save(sock, 0, 0, COLONY_WIDTH, COLONY_HEIGHT, "output/colony.png");
    }
    print("Closing connection");
    close(sock);
    return 0;
}