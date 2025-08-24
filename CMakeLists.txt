cmake_minimum_required(VERSION 3.17)
project(MarineGeneratorSimulator VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags - platform specific
if(MSVC)
    # MSVC flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od /Zi /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/Generator.cpp
    src/Sensors.cpp
    src/main.cpp
)

# Header files
set(HEADERS
    include/Generator.h
    include/Sensors.h
    include/SimpleJSON.h
)

# Create executable
add_executable(generator-simulator ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(generator-simulator
    Threads::Threads
)

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_link_libraries(generator-simulator ws2_32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
elseif(UNIX)
    # Unix/Linux-specific settings
    target_link_libraries(generator-simulator pthread)
endif()

# Install target
install(TARGETS generator-simulator
    RUNTIME DESTINATION bin
)

# Copy headers to install directory
install(FILES ${HEADERS}
    DESTINATION include/marine-generator-simulator
)

# Set properties
set_target_properties(generator-simulator PROPERTIES
    OUTPUT_NAME "generator-simulator"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Add custom target for running tests
add_custom_target(run-tests
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/generator-simulator --test
    DEPENDS generator-simulator
    COMMENT "Running generator simulator tests"
)

# Print configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Optional: Add nlohmann/json if available
find_package(nlohmann_json 3.2.0 QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(generator-simulator nlohmann_json::nlohmann_json)
    message(STATUS "Found nlohmann/json: ${nlohmann_json_VERSION}")
else()
    message(WARNING "nlohmann/json not found. Please install it or use the bundled version.")
    # You can add a bundled version here if needed
endif()

# Optional: Add Boost.Asio if available
find_package(Boost QUIET COMPONENTS system)
if(Boost_FOUND)
    target_link_libraries(generator-simulator Boost::system)
    message(STATUS "Found Boost.System: ${Boost_VERSION}")
else()
    message(STATUS "Boost not found, using standard sockets")
endif()
