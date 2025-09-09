#!/usr/bin/env python3
"""
Minimal Python bridge for debugging communication issues
"""

import json
import sys
import logging

# Configure logging to stderr so it doesn't interfere with stdout communication
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(levelname)s - %(message)s',
    stream=sys.stderr
)
logger = logging.getLogger(__name__)

def main():
    logger.info("Debug Python bridge started")
    logger.info(f"Python version: {sys.version}")
    logger.info(f"stdin isatty: {sys.stdin.isatty()}")
    logger.info(f"stdout isatty: {sys.stdout.isatty()}")
    
    try:
        while True:
            logger.info("Waiting for input...")
            line = sys.stdin.readline()
            logger.info(f"Received line: '{line}' (length: {len(line)})")
            
            if not line:
                logger.info("EOF received, exiting")
                break
                
            line = line.strip()
            if not line:
                logger.info("Empty line, continuing")
                continue
                
            try:
                logger.info(f"Parsing JSON: {line}")
                request = json.loads(line)
                logger.info(f"Parsed request: {request}")
                
                # Create response
                response = {
                    "id": request.get("id", 0),
                    "success": True,
                    "message": f"Received {request.get('type', 'unknown')} request"
                }
                
                response_json = json.dumps(response)
                logger.info(f"Sending response: {response_json}")
                
                # Send response to stdout
                print(response_json, flush=True)
                logger.info("Response sent")
                
            except json.JSONDecodeError as e:
                logger.error(f"JSON decode error: {e}")
                error_response = {
                    "id": 0,
                    "success": False,
                    "error": f"JSON decode error: {e}"
                }
                print(json.dumps(error_response), flush=True)
                
    except Exception as e:
        logger.error(f"Unexpected error: {e}")
        import traceback
        logger.error(traceback.format_exc())

if __name__ == "__main__":
    main()
