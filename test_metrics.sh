#!/bin/bash

# Test script for comprehensive metrics logging
# Tests each file 5 times (GET + PUT) and calculates averages

# Configuration
TEST_FOLDER="./test_files"
SERVER_IP="127.0.0.1"
SERVER_PORT="9000"
OUTPUT_CSV="metrics_test_$(date +%s).csv"
ITERATIONS=5

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔═══════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║     Automated Metrics Testing Script             ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════╝${NC}"

# Check if test folder exists
if [ ! -d "$TEST_FOLDER" ]; then
    echo -e "${RED}Error: Test folder '$TEST_FOLDER' not found!${NC}"
    exit 1
fi

# Check if client executable exists
if [ ! -f "./build/client_test" ]; then
    echo -e "${RED}Error: Client executable not found. Build first with: cmake --build build${NC}"
    exit 1
fi

# Create CSV header
echo "STT,File Name,File Size (bytes),Operation,Packets,Throughput (Mbps),RTT (ms),Transfer Latency (ms),Packet Loss Rate (%)" > "$OUTPUT_CSV"

echo -e "${GREEN}✓ CSV file created: $OUTPUT_CSV${NC}"
echo -e "${BLUE}Test configuration:${NC}"
echo "  - Test folder: $TEST_FOLDER"
echo "  - Iterations per file: $ITERATIONS"
echo "  - Server: $SERVER_IP:$SERVER_PORT"
echo ""

# Find all files in test folder
files=($(find "$TEST_FOLDER" -type f))
total_files=${#files[@]}

if [ $total_files -eq 0 ]; then
    echo -e "${RED}Error: No files found in $TEST_FOLDER${NC}"
    exit 1
fi

echo -e "${GREEN}Found $total_files files to test${NC}"
echo ""

stt=1

# Test each file
for filepath in "${files[@]}"; do
    filename=$(basename "$filepath")
    filesize=$(stat -f%z "$filepath" 2>/dev/null || stat -c%s "$filepath" 2>/dev/null)
    
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${GREEN}Testing file [$stt/$total_files]: $filename (${filesize} bytes)${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    
    # Calculate number of packets (assuming 64KB chunks)
    chunk_size=65536
    packets=$(( (filesize + chunk_size - 1) / chunk_size ))
    
    # Arrays to store metrics for averaging
    put_throughput=()
    put_rtt=()
    put_latency=()
    put_loss=()
    
    get_throughput=()
    get_rtt=()
    get_latency=()
    get_loss=()
    
    # Test PUT operation
    echo -e "${BLUE}Testing PUT operation (upload)...${NC}"
    for i in $(seq 1 $ITERATIONS); do
        echo "  Iteration $i/$ITERATIONS..."
        
        # Create temporary metrics file
        temp_metrics=$(mktemp)
        
        # Execute PUT command and capture metrics
        ./build/client_test "$SERVER_IP" "$SERVER_PORT" << EOF > "$temp_metrics" 2>&1
put $filepath
metrics
quit
EOF
        
        # Parse metrics from output
        throughput=$(grep -oP "Throughput:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        rtt=$(grep -oP "RTT:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        latency=$(grep -oP "Transfer Latency:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        loss=$(grep -oP "Packet Loss Rate:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        
        # Store values (default to 0 if not found)
        put_throughput+=("${throughput:-0}")
        put_rtt+=("${rtt:-0}")
        put_latency+=("${latency:-0}")
        put_loss+=("${loss:-0}")
        
        rm "$temp_metrics"
        sleep 0.5
    done
    
    # Test GET operation
    echo -e "${BLUE}Testing GET operation (download)...${NC}"
    for i in $(seq 1 $ITERATIONS); do
        echo "  Iteration $i/$ITERATIONS..."
        
        # Create temporary metrics file
        temp_metrics=$(mktemp)
        download_dir=$(mktemp -d)
        
        # Execute GET command and capture metrics
        ./build/simp_client "$SERVER_IP" "$SERVER_PORT" << EOF > "$temp_metrics" 2>&1
get $filename $download_dir
metrics
quit
EOF
        
        # Parse metrics from output
        throughput=$(grep -oP "Throughput:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        rtt=$(grep -oP "RTT:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        latency=$(grep -oP "Transfer Latency:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        loss=$(grep -oP "Packet Loss Rate:\s+\K[0-9.]+" "$temp_metrics" | tail -1)
        
        # Store values (default to 0 if not found)
        get_throughput+=("${throughput:-0}")
        get_rtt+=("${rtt:-0}")
        get_latency+=("${latency:-0}")
        get_loss+=("${loss:-0}")
        
        rm -rf "$temp_metrics" "$download_dir"
        sleep 0.5
    done
    
    # Calculate averages
    avg_put_throughput=$(echo "${put_throughput[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    avg_put_rtt=$(echo "${put_rtt[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    avg_put_latency=$(echo "${put_latency[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    avg_put_loss=$(echo "${put_loss[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    
    avg_get_throughput=$(echo "${get_throughput[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    avg_get_rtt=$(echo "${get_rtt[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    avg_get_latency=$(echo "${get_latency[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    avg_get_loss=$(echo "${get_loss[@]}" | awk '{s=0; for(i=1;i<=NF;i++)s+=$i; print s/NF}')
    
    # Write to CSV
    echo "$stt,$filename,$filesize,PUT,$packets,$avg_put_throughput,$avg_put_rtt,$avg_put_latency,$avg_put_loss" >> "$OUTPUT_CSV"
    echo "$((stt+1)),$filename,$filesize,GET,$packets,$avg_get_throughput,$avg_get_rtt,$avg_get_latency,$avg_get_loss" >> "$OUTPUT_CSV"
    
    # Display summary
    echo -e "${GREEN}✓ PUT: Throughput=${avg_put_throughput} Mbps, RTT=${avg_put_rtt} ms, Latency=${avg_put_latency} ms${NC}"
    echo -e "${GREEN}✓ GET: Throughput=${avg_get_throughput} Mbps, RTT=${avg_get_rtt} ms, Latency=${avg_get_latency} ms${NC}"
    echo ""
    
    stt=$((stt+2))
done

echo -e "${BLUE}╔═══════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║           Testing Completed!                      ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════╝${NC}"
echo -e "${GREEN}Results saved to: $OUTPUT_CSV${NC}"
echo -e "${GREEN}Total files tested: $total_files${NC}"
echo -e "${GREEN}Total operations: $((total_files * 2))${NC}"
echo ""
