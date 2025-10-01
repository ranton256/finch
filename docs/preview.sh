#!/bin/bash
# Local preview server for GitHub Pages documentation

PORT=${1:-8000}

echo "Starting local documentation server on port $PORT..."
echo "Visit: http://localhost:$PORT"
echo "Press Ctrl+C to stop"
echo ""

# Try different server options in order of preference
if command -v python3 &> /dev/null; then
    echo "Using Python 3 http.server"
    cd "$(dirname "$0")"
    python3 -m http.server "$PORT"
elif command -v python &> /dev/null; then
    echo "Using Python 2 SimpleHTTPServer"
    cd "$(dirname "$0")"
    python -m SimpleHTTPServer "$PORT"
else
    echo "Error: No suitable server found."
    echo "Please install Python 3 or Python 2"
    echo ""
    echo "Or install Node.js and run:"
    echo "  npx http-server docs -p $PORT"
    exit 1
fi
