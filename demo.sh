#!/bin/bash

# Script để demo hệ thống file transfer
# Chạy script này để xem demo tự động

echo "=================================="
echo "File Transfer System Demo"
echo "=================================="
echo ""

# Kiểm tra xem đã build chưa
if [ ! -f "build/server_test" ] || [ ! -f "build/client_test" ]; then
    echo "[INFO] Building project..."
    cd build
    cmake .. && make
    cd ..
    echo ""
fi

# Tạo thư mục shared nếu chưa có
mkdir -p shared
mkdir -p downloads

echo "[STEP 1] Starting server in background..."
./build/server_test 8080 ./shared > server.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"
sleep 2

echo ""
echo "[STEP 2] Creating test files..."

# Tạo file test để upload
cat > upload_test.txt << EOF
This is a test file created by the demo script.
Current time: $(date)
Random data: $RANDOM

This file will be uploaded to the server.
EOF

echo "Created: upload_test.txt"

echo ""
echo "[STEP 3] Testing file operations..."
echo ""

# Tạo file script cho client
cat > client_commands.txt << EOF
connect 127.0.0.1 8080
list
get test_file.txt downloads/
put upload_test.txt
list
metrics
disconnect
quit
EOF

echo "[INFO] Running client with automated commands..."
echo "Commands:"
cat client_commands.txt
echo ""

# Không thể tự động input vào interactive shell dễ dàng
# Thay vào đó, hướng dẫn người dùng

echo "=================================="
echo "Manual Testing Required"
echo "=================================="
echo ""
echo "Server is running with PID: $SERVER_PID"
echo ""
echo "In a new terminal, run:"
echo "  ./build/client_test 127.0.0.1 8080"
echo ""
echo "Then try these commands:"
echo "  1. list                      # List files on server"
echo "  2. get test_file.txt         # Download test file"
echo "  3. put upload_test.txt       # Upload test file"
echo "  4. metrics                   # View statistics"
echo "  5. quit                      # Exit client"
echo ""
echo "Press Enter to stop the server and clean up..."
read

echo ""
echo "[CLEANUP] Stopping server (PID: $SERVER_PID)..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo "[INFO] Cleaning up temporary files..."
rm -f client_commands.txt
rm -f server.log

echo ""
echo "=================================="
echo "Demo completed!"
echo "=================================="
