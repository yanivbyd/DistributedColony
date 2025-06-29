#include "image_saver.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "external/stb_image_write.h"
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <iostream>

// For PNG support, we'll use a simple implementation
// In a real project, you might want to use a library like libpng or stb_image_write

void ImageSaver::ensure_output_directory(const std::string& filename) {
    size_t last_slash = filename.find_last_of('/');
    if (last_slash != std::string::npos) {
        std::string dir = filename.substr(0, last_slash);
        struct stat st = {0};
        if (stat(dir.c_str(), &st) == -1) {
            mkdir(dir.c_str(), 0755);
        }
    }
}

bool ImageSaver::save_bmp(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb_data) {
    ensure_output_directory(filename);
    
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
            bmpfile[dst_idx + 0] = rgb_data[src_idx + 2]; // Blue
            bmpfile[dst_idx + 1] = rgb_data[src_idx + 1]; // Green
            bmpfile[dst_idx + 2] = rgb_data[src_idx + 0]; // Red
        }
    }
    
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return false;
    }
    
    ofs.write(reinterpret_cast<const char*>(bmpfile.data()), bmpfile.size());
    return ofs.good();
}

bool ImageSaver::save_png(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb_data) {
    ensure_output_directory(filename);
    
    // Use stb_image_write for proper PNG support
    int result = stbi_write_png(filename.c_str(), width, height, 3, rgb_data.data(), width * 3);
    
    if (result == 0) {
        std::cerr << "Failed to save PNG file: " << filename << std::endl;
        return false;
    }
    
    return true;
}

bool ImageSaver::save_image(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb_data) {
    // Determine format based on file extension
    std::string lower_filename = filename;
    std::transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(), ::tolower);
    
    if (lower_filename.ends_with(".bmp")) {
        return save_bmp(filename, width, height, rgb_data);
    } else if (lower_filename.ends_with(".png")) {
        return save_png(filename, width, height, rgb_data);
    } else {
        // Default to PNG if no extension or unknown extension
        std::string png_filename = filename;
        if (!png_filename.ends_with(".png")) {
            png_filename += ".png";
        }
        return save_png(png_filename, width, height, rgb_data);
    }
} 