#ifndef FO_API_H
#define FO_API_H

#include "../build/be_api/colony.pb.h"
#include <string>

void print(const std::string& msg);
int create_and_connect_socket(const char* ip, int port);
void ping_backend(int sock);
void init_colony(int sock, int width, int height);
void blast(int sock, int x, int y, int radius, distributedcolony::Color color);
void blast(int sock, int x, int y, int radius);
void get_image_and_save(int sock, int offsetX, int offsetY, int width, int height, const std::string& filename, bool quiet = false);

#endif // FO_API_H 