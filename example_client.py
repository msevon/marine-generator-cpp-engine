#!/usr/bin/env python3
"""
Example client for the Marine Generator Simulator Engine

This script demonstrates how to connect to and control the C++ engine
over TCP sockets using the JSON communication protocol.
"""

import socket
import json
import time
import sys

class GeneratorClient:
    def __init__(self, host='localhost', port=8081):
        self.host = host
        self.port = port
        self.socket = None
        
    def connect(self):
        """Connect to the generator engine"""
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(5.0)
            self.socket.connect((self.host, self.port))
            print(f"✅ Connected to generator engine at {self.host}:{self.port}")
            return True
        except Exception as e:
            print(f"❌ Failed to connect: {e}")
            return False
    
    def send_command(self, command):
        """Send a command to the engine and get response"""
        if not self.socket:
            print("❌ Not connected")
            return None
            
        try:
            print(f"📤 Sending: {command}")
            self.socket.send(command.encode('utf-8'))
            
            response = self.socket.recv(1024).decode('utf-8')
            print(f"📥 Response: {response}")
            
            try:
                return json.loads(response)
            except json.JSONDecodeError:
                print("⚠️  Response is not valid JSON")
                return response
                
        except Exception as e:
            print(f"❌ Error sending command: {e}")
            return None
    
    def close(self):
        """Close the connection"""
        if self.socket:
            self.socket.close()
            print("🔌 Connection closed")

def main():
    """Main demonstration function"""
    print("🚢 Marine Generator Simulator - Example Client")
    print("=" * 50)
    
    # Create client and connect
    client = GeneratorClient()
    if not client.connect():
        print("❌ Cannot continue without connection")
        sys.exit(1)
    
    try:
        # Get initial status
        print("\n📊 Getting initial status...")
        status = client.send_command("status")
        
        # Start the generator
        print("\n🚀 Starting generator...")
        result = client.send_command("start")
        
        # Wait for startup
        print("\n⏳ Waiting for startup...")
        time.sleep(3)
        
        # Check status again
        print("\n📊 Checking status after startup...")
        status = client.send_command("status")
        
        # Set load to 50%
        print("\n⚡ Setting load to 50%...")
        result = client.send_command("set_load 50")
        
        # Wait and check status
        print("\n⏳ Waiting for load change...")
        time.sleep(2)
        
        print("\n📊 Final status check...")
        status = client.send_command("status")
        
        # Demonstrate load restrictions
        print("\n🔒 Testing load restrictions...")
        print("Trying to set load to 10% (should be rejected)...")
        result = client.send_command("set_load 10")
        
        # Stop the generator
        print("\n🛑 Stopping generator...")
        result = client.send_command("stop")
        
        # Final status
        print("\n📊 Final status...")
        status = client.send_command("status")
        
        print("\n🎉 Demonstration completed!")
        
    except KeyboardInterrupt:
        print("\n\n⏹️  Interrupted by user")
    except Exception as e:
        print(f"\n❌ Error during demonstration: {e}")
    finally:
        client.close()

if __name__ == "__main__":
    main()
