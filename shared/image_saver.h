#ifndef IMAGE_SAVER_H
#define IMAGE_SAVER_H

#include <string>
#include <vector>
#include <cstdint>

class ImageSaver {
public:
    // Save RGB data as BMP file
    static bool save_bmp(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb_data);
    
    // Save RGB data as PNG file
    static bool save_png(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb_data);
    
    // Save RGB data with automatic format detection based on file extension
    static bool save_image(const std::string& filename, int width, int height, const std::vector<uint8_t>& rgb_data);
    
private:
    // Ensure output directory exists
    static void ensure_output_directory(const std::string& filename);
};

#endif // IMAGE_SAVER_H 