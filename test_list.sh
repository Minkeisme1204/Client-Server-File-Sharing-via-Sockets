#!/bin/bash

# Script to test file transfer
echo "Starting server..."
cd /home/minkeisrtx5090/Desktop/Workplace/HUST/06_Computer_Network/Client-Server-File-Sharing-via-Sockets

# Start server in background
./build/server_test 8080 ./shared > /tmp/server.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

sleep 2

echo ""
echo "Testing client LIST command..."
echo "list" | timeout 5 ./build/client_test 127.0.0.1 8080

echo ""
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null

echo ""
echo "Server log:"
cat /tmp/server.log

rm -f /tmp/server.log
