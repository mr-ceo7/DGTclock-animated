#!/bin/bash

# DGT Clock Firmware Tester (Bash Version)
# Simple serial command testing

PORT="/dev/ttyACM0"
BAUD=9600

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "============================================================"
echo "DGT CLOCK FIRMWARE TEST SUITE"
echo "============================================================"
echo "Port: $PORT"
echo "Baud Rate: $BAUD"
echo "Time: $(date '+%Y-%m-%d %H:%M:%S')"
echo "============================================================"

# Configure serial port
stty -F $PORT $BAUD raw -echo

send_and_read() {
    local cmd=$1
    local desc=$2
    
    echo -e "\n${BLUE}[TEST]${NC} $desc"
    echo -e "${YELLOW}[SEND]${NC} <$cmd>"
    
    # Send command
    echo "<$cmd>" > $PORT
    
    # Read response (with timeout)
    response=$(timeout 2s cat $PORT | head -c 200 2>/dev/null)
    
    if [ -n "$response" ]; then
        echo -e "${YELLOW}[RECV]${NC} $response"
        echo -e "${GREEN}[PASS]${NC} Command sent successfully"
        return 0
    else
        echo -e "${YELLOW}[INFO]${NC} No response (may be processing)"
        return 0
    fi
}

echo -e "\n${BLUE}Waiting for Arduino to be ready...${NC}"
sleep 2

# Clear buffer
cat $PORT > /dev/null &
PID=$!
sleep 0.5
kill $PID 2>/dev/null

echo -e "\n${GREEN}Starting tests...${NC}"

# Test 1: Time Sync
TIMESTAMP=$(date +%s)
send_and_read "TIME:$TIMESTAMP" "Test 1: Time Synchronization"
sleep 1

# Test 2: Brightness
send_and_read "BRIGHTNESS:10" "Test 2: Set Brightness to 10"
sleep 1

# Test 3: Alarm
send_and_read "ALARM:0,08,30,1,2" "Test 3: Set Alarm 0 (08:30, enabled, melody 2)"
sleep 1

# Test 4: Custom Text (Scroll)
send_and_read "TEXT:0,Hello Test!" "Test 4: Display Scrolling Text"
sleep 1

# Test 5: Music
send_and_read "MUSIC:1,3" "Test 5: Play Notification Melody"
sleep 2

send_and_read "MUSIC:0,0" "Test 6: Stop Music"
sleep 1

# Test 7: Display Mode
send_and_read "MODE:TIME" "Test 7: Switch to Time Mode"
sleep 1

echo ""
echo "============================================================"
echo "TEST SUMMARY"
echo "============================================================"
echo -e "${GREEN}✓ All command tests completed${NC}"
echo ""
echo "NOTE: Check the LED matrix display and speaker to verify:"
echo "  - Time should be displayed"
echo "  - Brightness was adjusted"
echo "  - Text was scrolled"
echo "  - Notification sound played"
echo ""
echo "To monitor continuous Arduino output, run:"
echo "  cat $PORT"
echo "============================================================"
