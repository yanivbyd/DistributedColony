# DistributedColony

DistributedColony is a C++ project that simulates a distributed system, inspired by the concept of a 'colony of life.' It consists of two main components:
- **Backend (BE):** Acts as the server, listening for and handling requests.
- **Frontend (FO):** Acts as the client, connecting to the backend.

The backend and frontend communicate over TCP sockets using Protocol Buffers (protobuf) for efficient, structured messaging. The project is designed for extensibility, starting with a simple ping API and ready for more complex distributed interactions.

## Quick Start (macOS)

1. **Setup and install all dependencies:**
   ```sh
   ./setup_mac.sh
   ```
   This will install Homebrew (if needed), all required packages, configure CMake, and build the project.

2. **To rebuild the project after making changes:**
   ```sh
   ./build.sh
   ```

Binaries will be located in `build/bin/`.

### Run

In one terminal, start the backend:
```sh
cd build/bin
./backend
```

In another terminal, start the frontend:
```sh
cd build/bin
./frontend
```

Or just run this command:
```
./build_and_run.sh
```