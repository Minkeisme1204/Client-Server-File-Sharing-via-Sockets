#!/bin/bash

echo "Testing Graceful Shutdown"
echo "========================="
cd /home/minkeisrtx5090/Desktop/Workplace/HUST/06_Computer_Network/Client-Server-File-Sharing-via-Sockets

echo "Starting server..."
./build/server_test 8080 ./shared > /tmp/shutdown_test.log 2>&1 &
SERVER_PID=$!
echo "Server PID: $SERVER_PID"

sleep 2

echo ""
echo "Connecting client..."
echo -e "list\nquit" | ./build/client_test 127.0.0.1 8080 > /dev/null 2>&1

sleep 1

echo ""
echo "Sending SIGINT to server (Ctrl+C simulation)..."
kill -INT $SERVER_PID

sleep 2

echo ""
echo "Checking server status..."
if ps -p $SERVER_PID > /dev/null 2>&1; then
    echo "✗ Server still running (not good)"
    kill -9 $SERVER_PID 2>/dev/null
else
    echo "✓ Server terminated gracefully"
fi

echo ""
echo "Server shutdown log:"
tail -15 /tmp/shutdown_test.log

# Check for exception
if grep -q "terminate called" /tmp/shutdown_test.log; then
    echo ""
    echo "✗ FAILED: Exception during shutdown"
    exit 1
else
    echo ""
    echo "✓ SUCCESS: Clean shutdown, no exceptions"
    exit 0
fi
