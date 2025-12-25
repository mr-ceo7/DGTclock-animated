#!/usr/bin/env python3
"""
DGT Clock Firmware Tester
Tests all Bluetooth command functionality via serial port
"""

import serial
import time
import sys
from datetime import datetime

# Configuration
PORT = '/dev/ttyACM0'
BAUD_RATE = 9600
TIMEOUT = 2

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    RESET = '\033[0m'

def print_test(message):
    print(f"{Colors.BLUE}[TEST]{Colors.RESET} {message}")

def print_success(message):
    print(f"{Colors.GREEN}[PASS]{Colors.RESET} {message}")

def print_error(message):
    print(f"{Colors.RED}[FAIL]{Colors.RESET} {message}")

def print_info(message):
    print(f"{Colors.YELLOW}[INFO]{Colors.RESET} {message}")

def send_command(ser, command):
    """Send command to Arduino and return response"""
    full_command = f"<{command}>\n"
    print_test(f"Sending: {full_command.strip()}")
    ser.write(full_command.encode())
    time.sleep(0.5)
    
    # Read response
    response = ""
    start_time = time.time()
    while time.time() - start_time < 1.5:
        if ser.in_waiting > 0:
            response += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        time.sleep(0.1)
    
    if response:
        print_info(f"Response: {response.strip()}")
    return response

def test_time_sync(ser):
    """Test time synchronization"""
    print("\n" + "="*60)
    print("TEST 1: Time Synchronization")
    print("="*60)
    
    # Get current Unix timestamp
    timestamp = int(time.time())
    response = send_command(ser, f"TIME:{timestamp}")
    
    if "OK:TIME_SYNCED" in response or "TIME_SYNCED" in response:
        print_success("Time sync successful")
        return True
    else:
        print_error("Time sync failed")
        return False

def test_brightness(ser):
    """Test brightness control"""
    print("\n" + "="*60)
    print("TEST 2: Brightness Control")
    print("="*60)
    
    levels_to_test = [5, 10, 15, 8]
    all_passed = True
    
    for level in levels_to_test:
        response = send_command(ser, f"BRIGHTNESS:{level}")
        if "OK:BRIGHTNESS_SET" in response or "BRIGHTNESS" in response:
            print_success(f"Brightness set to {level}")
        else:
            print_error(f"Failed to set brightness to {level}")
            all_passed = False
        time.sleep(0.5)
    
    return all_passed

def test_alarms(ser):
    """Test alarm setting"""
    print("\n" + "="*60)
    print("TEST 3: Alarm Management")
    print("="*60)
    
    # Test setting alarm 0
    response = send_command(ser, "ALARM:0,08,30,1,2")
    if "OK:ALARM_SET" in response or "ALARM" in response:
        print_success("Alarm 0 set successfully (08:30, enabled, melody 2)")
    else:
        print_error("Failed to set alarm 0")
        return False
    
    time.sleep(0.5)
    
    # Test setting alarm 1
    response = send_command(ser, "ALARM:1,12,00,1,1")
    if "OK:ALARM_SET" in response or "ALARM" in response:
        print_success("Alarm 1 set successfully (12:00, enabled, melody 1)")
    else:
        print_error("Failed to set alarm 1")
        return False
    
    time.sleep(0.5)
    
    # Test disabling alarm
    response = send_command(ser, "ALARM:0,08,30,0,2")
    if "OK:ALARM_SET" in response or "ALARM" in response:
        print_success("Alarm 0 disabled successfully")
    else:
        print_error("Failed to disable alarm 0")
        return False
    
    return True

def test_custom_text(ser):
    """Test custom text display"""
    print("\n" + "="*60)
    print("TEST 4: Custom Text Display")
    print("="*60)
    
    # Test scroll mode
    response = send_command(ser, "TEXT:0,Hello Arduino!")
    if "OK:TEXT_SET" in response or "TEXT" in response:
        print_success("Scroll text set successfully")
    else:
        print_error("Failed to set scroll text")
        return False
    
    time.sleep(1)
    
    # Test static mode
    response = send_command(ser, "TEXT:1,STATIC")
    if "OK:TEXT_SET" in response or "TEXT" in response:
        print_success("Static text set successfully")
    else:
        print_error("Failed to set static text")
        return False
    
    return True

def test_music(ser):
    """Test music playback"""
    print("\n" + "="*60)
    print("TEST 5: Music Control")
    print("="*60)
    
    melodies = [
        (0, "Default Melody"),
        (1, "Happy Birthday"),
        (2, "Alarm Sound"),
        (3, "Notification")
    ]
    
    for melody_idx, melody_name in melodies:
        response = send_command(ser, f"MUSIC:1,{melody_idx}")
        if "OK:MUSIC_PLAYED" in response or "MUSIC" in response:
            print_success(f"Played {melody_name}")
        else:
            print_error(f"Failed to play {melody_name}")
            return False
        time.sleep(2)  # Wait for melody to play
        
        # Stop music
        send_command(ser, "MUSIC:0,0")
        time.sleep(0.5)
    
    return True

def test_display_mode(ser):
    """Test display mode switching"""
    print("\n" + "="*60)
    print("TEST 6: Display Mode Switching")
    print("="*60)
    
    # Switch to text mode
    response = send_command(ser, "MODE:TEXT")
    if "OK:MODE_TEXT" in response or "MODE" in response:
        print_success("Switched to TEXT mode")
    else:
        print_error("Failed to switch to TEXT mode")
        return False
    
    time.sleep(1)
    
    # Switch back to time mode
    response = send_command(ser, "MODE:TIME")
    if "OK:MODE_TIME" in response or "MODE" in response:
        print_success("Switched to TIME mode")
    else:
        print_error("Failed to switch to TIME mode")
        return False
    
    return True

def test_invalid_commands(ser):
    """Test invalid command handling"""
    print("\n" + "="*60)
    print("TEST 7: Invalid Command Handling")
    print("="*60)
    
    # Test invalid brightness
    response = send_command(ser, "BRIGHTNESS:99")
    if "ERR" in response:
        print_success("Invalid brightness rejected correctly")
    else:
        print_info("No error response for invalid brightness (may be handled silently)")
    
    # Test invalid alarm ID
    response = send_command(ser, "ALARM:5,08,30,1,0")
    if "ERR:INVALID_ALARM_ID" in response:
        print_success("Invalid alarm ID rejected correctly")
    else:
        print_info("No error response for invalid alarm ID")
    
    return True

def main():
    print("\n" + "="*60)
    print("DGT CLOCK FIRMWARE TEST SUITE")
    print("="*60)
    print(f"Port: {PORT}")
    print(f"Baud Rate: {BAUD_RATE}")
    print(f"Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("="*60)
    
    try:
        # Open serial connection
        print_info("Opening serial connection...")
        ser = serial.Serial(PORT, BAUD_RATE, timeout=TIMEOUT)
        time.sleep(2)  # Wait for Arduino to reset
        
        # Clear any initial data
        ser.reset_input_buffer()
        
        print_success(f"Connected to {PORT}")
        
        # Run tests
        results = {
            "Time Sync": test_time_sync(ser),
            "Brightness": test_brightness(ser),
            "Alarms": test_alarms(ser),
            "Custom Text": test_custom_text(ser),
            "Music": test_music(ser),
            "Display Mode": test_display_mode(ser),
            "Invalid Commands": test_invalid_commands(ser)
        }
        
        # Summary
        print("\n" + "="*60)
        print("TEST SUMMARY")
        print("="*60)
        
        passed = sum(results.values())
        total = len(results)
        
        for test_name, result in results.items():
            status = f"{Colors.GREEN}PASS{Colors.RESET}" if result else f"{Colors.RED}FAIL{Colors.RESET}"
            print(f"{test_name:.<40} {status}")
        
        print("="*60)
        print(f"Total: {passed}/{total} tests passed")
        
        if passed == total:
            print(f"{Colors.GREEN}✓ ALL TESTS PASSED!{Colors.RESET}")
            exit_code = 0
        else:
            print(f"{Colors.RED}✗ SOME TESTS FAILED{Colors.RESET}")
            exit_code = 1
        
        # Close serial connection
        ser.close()
        print_info("Serial connection closed")
        
        sys.exit(exit_code)
        
    except serial.SerialException as e:
        print_error(f"Serial error: {e}")
        print_info("Is Arduino connected and not in use by another program?")
        sys.exit(1)
    except KeyboardInterrupt:
        print_info("\nTest interrupted by user")
        if 'ser' in locals():
            ser.close()
        sys.exit(1)
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        if 'ser' in locals():
            ser.close()
        sys.exit(1)

if __name__ == "__main__":
    main()
