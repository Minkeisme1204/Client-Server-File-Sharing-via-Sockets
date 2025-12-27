#!/bin/bash

echo "Testing FILE UPLOAD (PUT)"
echo "=========================="
cd /home/minkeisrtx5090/Desktop/Workplace/HUST/06_Computer_Network/Client-Server-File-Sharing-via-Sockets

# Create test file
echo "Creating test upload file..."
cat > /tmp/upload_test.txt << 'EOF'
This is a test file for upload.
Testing PUT command.
Line 3
Line 4
Line 5
EOF

# Start server
echo "Starting server..."
./build/server_test 8080 ./shared > /tmp/server_put.log 2>&1 &
SERVER_PID=$!
sleep 2

# Test upload
echo ""
echo "Uploading file..."
echo -e "put /tmp/upload_test.txt\nquit" | ./build/client_test 127.0.0.1 8080

echo ""
echo "Checking if file was uploaded..."
ls -lh ./shared/upload_test.txt 2>/dev/null && echo "✓ File uploaded successfully!" || echo "✗ File not found"

echo ""
echo "Content of uploaded file:"
cat ./shared/upload_test.txt 2>/dev/null

# Stop server
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

# Cleanup
rm -f /tmp/upload_test.txt
