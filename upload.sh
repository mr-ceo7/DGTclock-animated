#!/bin/bash

# DGT Clock Upload Script
# This script will automatically detect and upload firmware to Arduino Uno R3

export PATH="$HOME/.local/bin:$PATH"

echo "🔍 Searching for Arduino Uno R3..."
echo ""

# Detect Arduino board
BOARD_INFO=$(arduino-cli board list)

if echo "$BOARD_INFO" | grep -q "arduino:avr:uno"; then
    echo "✅ Arduino Uno detected!"
    PORT=$(echo "$BOARD_INFO" | grep "arduino:avr:uno" | awk '{print $1}')
    echo "📍 Port: $PORT"
    echo ""
    echo "⚡ Uploading firmware..."
    echo ""
    
    arduino-cli upload -p $PORT --fqbn arduino:avr:uno DGTclock-animated.ino
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "✅ Upload successful!"
        echo "🎉 Your DGT Clock is now running the enhanced Bluetooth firmware!"
        echo ""
        echo "Next steps:"
        echo "1. Open Serial Monitor: arduino-cli monitor -p $PORT -c baudrate=9600"
        echo "2. Pair HC-05 Bluetooth module with your device"
        echo "3. Open the web dashboard and connect"
    else
        echo ""
        echo "❌ Upload failed. Please check:"
        echo "   - Arduino is properly connected"
        echo "   - No other program is using the serial port"
        echo "   - You have permissions to access the port"
    fi
else
    echo "❌ No Arduino Uno detected"
    echo ""
    echo "Please:"
    echo "1. Connect your Arduino Uno R3 via USB cable"
    echo "2. Wait a few seconds for the system to recognize it"
    echo "3. Run this script again"
    echo ""
    echo "Current detected devices:"
    arduino-cli board list
fi
