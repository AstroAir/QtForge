#!/usr/bin/env python3
"""
Test script to debug Python bridge communication
"""

import json
import sys

def test_communication():
    print("Python bridge test started", flush=True)
    
    try:
        # Read from stdin
        line = sys.stdin.readline()
        print(f"Received line: '{line}'", flush=True)
        
        if line.strip():
            try:
                request = json.loads(line.strip())
                print(f"Parsed request: {request}", flush=True)
                
                # Send response
                response = {
                    "id": request.get("id", 0),
                    "success": True,
                    "message": "Test response"
                }
                response_json = json.dumps(response)
                print(response_json, flush=True)
                
            except json.JSONDecodeError as e:
                print(f"JSON decode error: {e}", flush=True)
        else:
            print("Empty line received", flush=True)
            
    except Exception as e:
        print(f"Error: {e}", flush=True)

if __name__ == "__main__":
    test_communication()
