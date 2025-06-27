#!/bin/bash
set -e

# Kill any running backend/frontend
pkill -f build/bin/backend || true
pkill -f build/bin/frontend || true

echo -e "\033[1;35m================[ BUILD ] ================\033[0m"

./build.sh

echo -e "\033[1;35m================[ RUN ] --================\033[0m"

# Run backend in the background
build/bin/backend &

# Run frontend in the foreground
build/bin/frontend 