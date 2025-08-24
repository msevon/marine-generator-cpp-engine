# Marine Generator Simulator - Communication Protocol

This document describes the TCP socket communication protocol used by the Marine Generator Simulator Engine.

## Overview

The engine runs a TCP server on port 8081 that accepts text commands and responds with JSON data. The protocol is designed to be simple and human-readable while providing comprehensive generator control and monitoring capabilities.

## Connection

- **Protocol**: TCP
- **Port**: 8081
- **Address**: localhost (or the machine's IP address)
- **Encoding**: UTF-8

## Commands

All commands are sent as plain text strings terminated with a newline character.

### Available Commands

| Command | Description | Parameters | Example |
|---------|-------------|------------|---------|
| `start` | Start the generator | None | `start` |
| `stop` | Stop the generator normally | None | `stop` |
| `emergency_stop` | Emergency shutdown | None | `emergency_stop` |
| `set_load` | Set generator load | Percentage (0-100) | `set_load 75` |
| `status` | Get current status | None | `status` |

### Command Details

#### Start Command
```
start
```
- **Effect**: Initiates generator startup sequence
- **State Transition**: STOPPED → STARTING → RUNNING
- **Response**: Success/failure message
- **Notes**: Generator will automatically transition through startup states

#### Stop Command
```
stop
```
- **Effect**: Initiates normal shutdown sequence
- **State Transition**: RUNNING → STOPPING → STOPPED
- **Response**: Success/failure message
- **Notes**: Graceful shutdown with proper cooldown

#### Emergency Stop Command
```
emergency_stop
```
- **Effect**: Immediate shutdown (bypasses normal sequence)
- **State Transition**: Any → STOPPED
- **Response**: Success/failure message
- **Notes**: Use only in critical situations

#### Set Load Command
```
set_load <percentage>
```
- **Effect**: Changes generator load output
- **Parameters**: 
  - `percentage`: Integer or float (0-100)
- **Restrictions**: 
  - Cannot change when stopped
  - Minimum 20% when running
- **Response**: Success/failure message
- **Example**: `set_load 75`

#### Status Command
```
status
```
- **Effect**: Returns current generator status
- **Response**: JSON object with all sensor data
- **Update Rate**: Real-time (reflects current simulation state)

## Responses

All commands return a response in JSON format.

### Success Response Format
```json
{
  "status": "success",
  "data": {
    "state": 2,
    "rpm": 1800.0,
    "voltage": 440.0,
    "frequency": 60.0,
    "load": 75.0,
    "fuel_level": 100.0,
    "oil_pressure": 45.0,
    "cooling_temp": 85.0,
    "alarms": []
  }
}
```

### Error Response Format
```json
{
  "status": "error",
  "message": "Cannot change load - generator is stopped"
}
```

## Data Fields

### State Values
| Value | State | Description |
|-------|-------|-------------|
| 0 | STOPPED | Generator is off |
| 1 | STARTING | Generator is starting up |
| 2 | RUNNING | Generator is running normally |
| 3 | STOPPING | Generator is shutting down |
| 4 | FAULT | Generator has encountered an error |

### Sensor Data
| Field | Unit | Range | Description |
|-------|------|-------|-------------|
| `rpm` | RPM | 0-2000 | Generator rotational speed |
| `voltage` | V | 0-500 | Output voltage |
| `frequency` | Hz | 0-70 | Output frequency |
| `load` | % | 0-100 | Current load percentage |
| `fuel_level` | % | 0-100 | Remaining fuel level |
| `oil_pressure` | PSI | 0-100 | Engine oil pressure |
| `cooling_temp` | °C | 0-120 | Cooling water temperature |

### Alarms
The `alarms` field contains an array of active alarm objects:
```json
{
  "type": "high_temperature",
  "message": "Cooling temperature above normal",
  "severity": "warning",
  "timestamp": "2024-01-01T12:00:00Z"
}
```

## Error Handling

### Common Error Scenarios
1. **Invalid Command**: Unknown command sent
2. **Invalid Parameters**: Command parameters out of range
3. **State Restrictions**: Command not allowed in current state
4. **Connection Issues**: Network or socket problems

### Error Response Examples
```json
{
  "status": "error",
  "message": "Unknown command: invalid_cmd"
}
```

```json
{
  "status": "error", 
  "message": "Load must be between 0 and 100"
}
```

```json
{
  "status": "error",
  "message": "Cannot change load - generator is stopped"
}
```

## Usage Examples

### Basic Control Sequence
```python
import socket

# Connect to engine
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 8081))

# Start generator
sock.send(b'start\n')
response = sock.recv(1024).decode()

# Set load
sock.send(b'set_load 75\n')
response = sock.recv(1024).decode()

# Get status
sock.send(b'status\n')
response = sock.recv(1024).decode()

# Stop generator
sock.send(b'stop\n')
response = sock.recv(1024).decode()

sock.close()
```

### Monitoring Loop
```python
import socket
import time
import json

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(('localhost', 8081))

try:
    while True:
        sock.send(b'status\n')
        response = sock.recv(1024).decode()
        data = json.loads(response)
        
        if data['status'] == 'success':
            state = data['data']['state']
            rpm = data['data']['rpm']
            print(f"State: {state}, RPM: {rpm}")
        
        time.sleep(1.0)
        
except KeyboardInterrupt:
    print("Monitoring stopped")
finally:
    sock.close()
```

## Implementation Notes

- **Thread Safety**: The engine handles multiple concurrent connections
- **Command Buffering**: Commands are processed in order
- **Response Timing**: Responses are sent immediately after command processing
- **Connection Limits**: No artificial connection limits (limited by system resources)
- **Keep-Alive**: Connections remain open until explicitly closed by client

## Testing

Test the protocol using any TCP client:
- **Netcat**: `echo "status" | nc localhost 8081`
- **Telnet**: `telnet localhost 8081`
- **Custom Scripts**: Use the provided `example_client.py`
- **Web Browsers**: Some browsers support raw TCP connections

## Troubleshooting

### Common Issues
1. **Connection Refused**: Engine not running or wrong port
2. **No Response**: Engine overloaded or crashed
3. **Invalid JSON**: Engine error or protocol mismatch
4. **Command Ignored**: Engine in wrong state for command

### Debug Steps
1. Verify engine is running (`netstat -an | findstr :8081`)
2. Check engine console output for errors
3. Test with simple commands first
4. Verify JSON response format
5. Check network firewall settings
