#!/bin/bash
# Quick start script for GUI applications

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}===== File Transfer GUI Applications =====${NC}"
echo ""

# Check if executables exist
if [ ! -f "./build/server_gui" ] || [ ! -f "./build/client_gui" ]; then
    echo "Executables not found. Building..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
fi

echo -e "${GREEN}Starting Server GUI...${NC}"
./build/server_gui &
SERVER_PID=$!

echo "Server GUI PID: $SERVER_PID"
echo ""

sleep 2

echo -e "${GREEN}Starting Client GUI...${NC}"
./build/client_gui &
CLIENT_PID=$!

echo "Client GUI PID: $CLIENT_PID"
echo ""

echo -e "${BLUE}Both GUI applications are now running!${NC}"
echo ""
echo "Usage:"
echo "1. In Server GUI: Configure port (default 9000) and shared directory (./shared)"
echo "2. Click 'Start Server' button"
echo "3. In Client GUI: Enter server IP (127.0.0.1) and port (9000)"
echo "4. Click 'Connect' button"
echo "5. Use 'List Files', 'Download File', 'Upload File' buttons"
echo ""
echo "Press Ctrl+C to stop monitoring (apps will continue running)"
echo ""

# Wait for both processes
wait $SERVER_PID $CLIENT_PID

echo ""
echo -e "${BLUE}Applications closed.${NC}"
