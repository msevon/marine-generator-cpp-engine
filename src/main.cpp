#include "Generator.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <cstring>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

class GeneratorServer {
private:
    Generator generator_;
    int server_socket_;
    int client_socket_;
    bool running_;
    std::thread simulation_thread_;
    
    static constexpr int PORT = 8081;
    static constexpr int BUFFER_SIZE = 1024;
    static constexpr double UPDATE_RATE = 200.0; // 200ms update rate

public:
    GeneratorServer() : server_socket_(-1), client_socket_(-1), running_(false) {}
    
    ~GeneratorServer() {
        stop();
    }
    
    bool initialize() {
        // Create socket
        server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
#ifdef _WIN32
        if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
#else
        if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
#endif
            std::cerr << "Failed to set socket options" << std::endl;
            return false;
        }
        
        // Bind socket
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);
        
        if (bind(server_socket_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to bind socket" << std::endl;
            return false;
        }
        
        // Listen for connections
        if (listen(server_socket_, 1) < 0) {
            std::cerr << "Failed to listen on socket" << std::endl;
            return false;
        }
        
        std::cout << "Generator server listening on port " << PORT << std::endl;
        return true;
    }
    
    void run() {
        running_ = true;
        
        // Start simulation thread
        simulation_thread_ = std::thread([this]() {
            simulation_loop();
        });
        
        // Accept client connections
        while (running_) {
            std::cout << "Waiting for client connection..." << std::endl;
            
            struct sockaddr_in client_addr;
#ifdef _WIN32
            int client_len = sizeof(client_addr);
#else
            socklen_t client_len = sizeof(client_addr);
#endif
            client_socket_ = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
            
            if (client_socket_ < 0) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }
            
#ifdef _WIN32
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "Client connected from " << client_ip << std::endl;
#else
            std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << std::endl;
#endif
            handle_client();
            
            // Client socket is closed in handle_client when it returns
            client_socket_ = -1;
        }
    }
    
    void stop() {
        running_ = false;
        
        if (simulation_thread_.joinable()) {
            simulation_thread_.join();
        }
        
        if (client_socket_ >= 0) {
            close(client_socket_);
            client_socket_ = -1;
        }
        
        if (server_socket_ >= 0) {
            close(server_socket_);
            server_socket_ = -1;
        }
    }

private:
    void simulation_loop() {
        auto last_update = std::chrono::high_resolution_clock::now();
        
        while (running_) {
            auto now = std::chrono::high_resolution_clock::now();
            auto delta_time = std::chrono::duration<double>(now - last_update).count();
            
            if (delta_time >= 1.0 / UPDATE_RATE) {
                generator_.update(delta_time);
                last_update = now;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // 5ms sleep
        }
    }
    
    void handle_client() {
        char buffer[BUFFER_SIZE];
        int bytes_received;
        
        std::cout << "Client connected, waiting for commands..." << std::endl;
        
        while (running_) {
            bytes_received = recv(client_socket_, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    std::cout << "Client disconnected" << std::endl;
                } else {
                    std::cout << "Error receiving data" << std::endl;
                }
                break;
            }
            
            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer << std::endl;
            
            // Simple command processing for now
            std::string command(buffer);
            std::string response;
            
            if (command.find("start") != std::string::npos) {
                generator_.start();
                response = "{\"status\":\"success\",\"message\":\"Generator started\"}";
            } else if (command.find("stop") != std::string::npos) {
                generator_.stop();
                response = "{\"status\":\"success\",\"message\":\"Generator stopped\"}";
            } else if (command.find("set_load") != std::string::npos) {
                // Parse load value from command (e.g., "set_load 75")
                size_t space_pos = command.find(' ');
                if (space_pos != std::string::npos) {
                    std::string load_str = command.substr(space_pos + 1);
                    try {
                        double load_value = std::stod(load_str);
                        if (load_value >= 0.0 && load_value <= 100.0) {
                            generator_.set_load(load_value);
                            response = "{\"status\":\"success\",\"message\":\"Load set to " + std::to_string(static_cast<int>(load_value)) + "%\"}";
                        } else {
                            response = "{\"status\":\"error\",\"message\":\"Load must be between 0 and 100\"}";
                        }
                    } catch (const std::exception& e) {
                        response = "{\"status\":\"error\",\"message\":\"Invalid load value\"}";
                    }
                } else {
                    response = "{\"status\":\"error\",\"message\":\"Missing load value\"}";
                }
            } else if (command.find("status") != std::string::npos) {
                auto status = generator_.get_status();
                response = "{\"status\":\"success\",\"data\":{\"state\":" + 
                          std::to_string(static_cast<int>(status.state)) + 
                          ",\"rpm\":" + std::to_string(status.rpm) + 
                          ",\"voltage\":" + std::to_string(status.voltage) + 
                          ",\"frequency\":" + std::to_string(status.frequency) + 
                          ",\"load\":" + std::to_string(status.load_percentage) + 
                          ",\"fuel_level\":" + std::to_string(status.fuel_level) + 
                          ",\"oil_pressure\":" + std::to_string(status.oil_pressure) + 
                          ",\"cooling_temp\":" + std::to_string(status.cooling_temp) + 
                          ",\"alarms\":[]}}";
            } else {
                response = "{\"status\":\"error\",\"message\":\"Unknown command\"}";
            }
            
            std::cout << "Sending response: " << response << std::endl;
            
            // Send response back to client
            int bytes_sent = send(client_socket_, response.c_str(), response.length(), 0);
            if (bytes_sent < 0) {
                std::cout << "Error sending response" << std::endl;
                break;
            }
        }
        
        // Close client socket when done
        close(client_socket_);
        std::cout << "Client connection closed" << std::endl;
    }
};

int main() {
    std::cout << "Marine Generator Simulator - C++ Engine" << std::endl;
    std::cout << "======================================" << std::endl;
    
#ifdef _WIN32
    // Initialize Windows Sockets
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }
    std::cout << "Windows Sockets initialized" << std::endl;
#endif
    
    GeneratorServer server;
    
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // Set up signal handling for graceful shutdown
#ifdef _WIN32
    // Windows doesn't have SIGINT, we'll handle Ctrl+C differently
    std::cout << "Press Ctrl+C to stop the server..." << std::endl;
#else
    signal(SIGINT, [](int) {
        std::cout << "\nShutting down server..." << std::endl;
        exit(0);
    });
#endif
    
    try {
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
