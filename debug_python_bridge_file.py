#!/usr/bin/env python3
"""
Minimal Python bridge for debugging communication issues - writes to file
"""

import json
import sys
import os
import time

# Write debug info to a file
debug_file = "python_bridge_debug.log"

def log_debug(message):
    with open(debug_file, "a", encoding="utf-8") as f:
        f.write(f"{time.strftime('%Y-%m-%d %H:%M:%S')} - {message}\n")
        f.flush()

def main():
    log_debug("Debug Python bridge started")
    log_debug(f"Python version: {sys.version}")
    log_debug(f"Working directory: {os.getcwd()}")
    log_debug(f"Script path: {__file__}")
    log_debug(f"stdin isatty: {sys.stdin.isatty()}")
    log_debug(f"stdout isatty: {sys.stdout.isatty()}")

    try:
        iteration = 0
        while True:
            iteration += 1
            log_debug(f"Iteration {iteration}: Waiting for input...")

            line = sys.stdin.readline()
            log_debug(f"Iteration {iteration}: Received line: '{line}' (length: {len(line)})")

            if not line:
                log_debug("EOF received, exiting")
                break

            line = line.strip()
            if not line:
                log_debug("Empty line, continuing")
                continue

            try:
                log_debug(f"Parsing JSON: {line}")
                request = json.loads(line)
                log_debug(f"Parsed request: {request}")

                # Create response
                response = {
                    "id": request.get("id", 0),
                    "success": True,
                    "message": f"Received {request.get('type', 'unknown')} request"
                }

                response_json = json.dumps(response)
                log_debug(f"Sending response: {response_json}")

                # Send response to stdout with aggressive flushing
                print(response_json, flush=True)
                sys.stdout.flush()  # Force flush
                log_debug("Response sent and flushed")

                # If this is a shutdown request, exit
                if request.get("type") == "shutdown":
                    log_debug("Shutdown requested, exiting")
                    break

            except json.JSONDecodeError as e:
                log_debug(f"JSON decode error: {e}")
                error_response = {
                    "id": 0,
                    "success": False,
                    "error": f"JSON decode error: {e}"
                }
                print(json.dumps(error_response), flush=True)

    except Exception as e:
        log_debug(f"Unexpected error: {e}")
        import traceback
        log_debug(traceback.format_exc())

    log_debug("Python bridge exiting")

if __name__ == "__main__":
    main()
