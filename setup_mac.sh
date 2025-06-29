#!/bin/bash
set -e

# Install Homebrew if not installed
echo "Checking for Homebrew..."
if ! command -v brew &>/dev/null; then
  echo "Homebrew not found. Installing Homebrew..."
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
fi

echo "Installing dependencies with Homebrew..."
brew update
brew install cmake protobuf abseil pkg-config

# Download stb library
echo "Downloading stb_image_write library..."
./download_stb.sh

# Configure and build the project
echo "Configuring project with CMake..."
cmake -Bbuild

echo "Building project..."
cmake --build build

echo "Setup complete! Binaries are in build/bin/"
