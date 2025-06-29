cmake_minimum_required(VERSION 3.10)
project(DistributedColony CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------------------- Protobuf Setup ----------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# Find threads package
find_package(Threads REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})
link_directories(${PROTOBUF_LIBRARY_DIRS})

find_program(PROTOC_EXECUTABLE protoc REQUIRED)

# Find all .proto files
file(GLOB PROTO_FILES "${CMAKE_SOURCE_DIR}/be_api/*.proto")

# Generate pb.cc/pb.h from each proto
set(GENERATED_SRCS "")
set(GENERATED_HDRS "")

foreach(PROTO_FILE ${PROTO_FILES})
    get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
    set(GEN_SRC "${CMAKE_BINARY_DIR}/be_api/${PROTO_NAME}.pb.cc")
    set(GEN_HDR "${CMAKE_BINARY_DIR}/be_api/${PROTO_NAME}.pb.h")

    list(APPEND GENERATED_SRCS ${GEN_SRC})
    list(APPEND GENERATED_HDRS ${GEN_HDR})

    add_custom_command(
        OUTPUT ${GEN_SRC} ${GEN_HDR}
        COMMAND ${PROTOC_EXECUTABLE}
        ARGS --cpp_out=${CMAKE_BINARY_DIR} -I ${CMAKE_SOURCE_DIR} ${PROTO_FILE}
        DEPENDS ${PROTO_FILE}
        COMMENT "Generating C++ protobuf: ${PROTO_NAME}"
        VERBATIM
    )
endforeach()

add_custom_target(generate_proto ALL DEPENDS ${GENERATED_SRCS} ${GENERATED_HDRS})

# ---------------------- Include Paths ----------------------
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/be_api)
include_directories(${CMAKE_SOURCE_DIR}/shared)

# ---------------------- Backend ----------------------
file(GLOB_RECURSE BACKEND_SOURCES "${CMAKE_SOURCE_DIR}/backend/*.cpp")
file(GLOB_RECURSE SHARED_SOURCES "${CMAKE_SOURCE_DIR}/shared/*.cpp")

add_executable(backend ${BACKEND_SOURCES} ${SHARED_SOURCES} ${GENERATED_SRCS})
add_dependencies(backend generate_proto)
target_link_libraries(backend ${PROTOBUF_LIBRARIES} Threads::Threads)

# ---------------------- Frontend ----------------------
file(GLOB_RECURSE FRONTEND_SOURCES "${CMAKE_SOURCE_DIR}/frontend/*.cpp")

add_executable(frontend ${FRONTEND_SOURCES} ${SHARED_SOURCES} ${GENERATED_SRCS})
add_dependencies(frontend generate_proto)
target_link_libraries(frontend ${PROTOBUF_LIBRARIES})
