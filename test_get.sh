#!/bin/bash

echo "Testing FILE DOWNLOAD (GET)"
echo "============================"
cd /home/minkeisrtx5090/Desktop/Workplace/HUST/06_Computer_Network/Client-Server-File-Sharing-via-Sockets

# Cleanup previous download
rm -f /tmp/downloaded_test.txt

# Start server
echo "Starting server..."
./build/server_test 8080 ./shared > /tmp/server_get.log 2>&1 &
SERVER_PID=$!
sleep 2

# Test download
echo ""
echo "Downloading test_file.txt..."
echo -e "get test_file.txt /tmp\nquit" | ./build/client_test 127.0.0.1 8080

echo ""
echo "Checking downloaded file..."
if [ -f /tmp/test_file.txt ]; then
    echo "✓ File downloaded successfully!"
    echo ""
    echo "File info:"
    ls -lh /tmp/test_file.txt
    echo ""
    echo "First 5 lines:"
    head -5 /tmp/test_file.txt
else
    echo "✗ File not found"
fi

# Stop server
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

# Cleanup
rm -f /tmp/test_file.txt
