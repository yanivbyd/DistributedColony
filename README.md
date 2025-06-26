# DistributedColony
A colony of life, running as a distributed system

## Building and Running

### Prerequisites
- You need a C++ compiler (e.g., g++) installed on your system.

### Compile

To compile the backend and frontend (binaries will be placed in the bin directory):

```sh
# Make sure bin directory exists
mkdir -p bin

# Compile backend
cd backend
g++ -o ../bin/backend main.cpp
cd ..

# Compile frontend
cd frontend
g++ -o ../bin/frontend main.cpp
cd ..
```

### Run

First, run the backend in one terminal:

```sh
./bin/backend
```

Then, in another terminal, run the frontend:

```sh
./bin/frontend
```

