# Changelog

All notable changes to the Marine Generator Simulator Engine will be documented in this file.

## [1.0.0] - 2024-01-01

### Added
- Complete generator state machine (STOPPED, STARTING, RUNNING, STOPPING, FAULT)
- Realistic sensor simulation with noise and drift
- TCP socket server for external communication
- JSON-based communication protocol
- Load management system with restrictions
- Alarm system for critical parameters
- Cross-platform compatibility (Windows, Linux, macOS)
- CMake build system
- Comprehensive documentation

### Features
- **Generator Control**: Start, stop, emergency stop commands
- **Load Management**: Dynamic load control (20-100% when running)
- **Sensor Simulation**: RPM, voltage, frequency, temperature, oil pressure, fuel level
- **Real-time Updates**: Continuous simulation loop
- **Network Communication**: TCP server on port 8081
- **Error Handling**: Comprehensive error responses and validation

### Technical Details
- **Language**: C++17
- **Dependencies**: Standard library only
- **Architecture**: Modular design with separate Generator and Sensors classes
- **Threading**: Multi-threaded simulation and communication
- **Protocol**: Simple text commands with JSON responses

### Documentation
- README.md with installation and usage instructions
- PROTOCOL.md with detailed communication protocol
- Example client implementation in Python
- Build scripts for Windows and Unix systems

## Future Versions

### Planned Features
- Multiple generator support
- Data logging and export
- Configuration file support
- Performance optimization
- Extended sensor types
- WebSocket support
- REST API interface

### Known Issues
- None currently identified

### Dependencies
- CMake 3.10+
- C++17 compatible compiler
- No external libraries required
