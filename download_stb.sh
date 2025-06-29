#!/bin/bash

# Download stb_image_write.h if it doesn't exist
if [ ! -f "shared/external/stb_image_write.h" ]; then
    echo "Downloading stb_image_write.h..."
    mkdir -p shared/external
    curl -o shared/external/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
    echo "Downloaded stb_image_write.h successfully!"
else
    echo "stb_image_write.h already exists."
fi 