# DGT Clock - Quick Setup Guide

## 📋 What You Need

### Hardware

- ✅ Arduino Uno R3
- ✅ HC-05 Bluetooth Module
- ✅ LED Matrix Display (4x MAX7219, 32x8)
- ✅ RTC DS3231 Module
- ✅ Speaker (connected to pin 3)
- ✅ Jumper wires

### Software

- ✅ Arduino IDE
- ✅ Chrome or Edge browser
- ✅ Vercel CLI (optional, for deployment)

---

## 🔌 Hardware Connections

### HC-05 Bluetooth Module

```
HC-05 VCC  → Arduino 5V
HC-05 GND  → Arduino GND
HC-05 TX   → Arduino Pin 7
HC-05 RX   → Arduino Pin 4 (use voltage divider: 1kΩ + 2kΩ)
```

### LED Matrix Display

```
CS Pin     → Arduino Pin 9
(Other connections as per LEDMatrixDriver library)
```

### RTC DS3231

```
SDA        → Arduino A4
SCL        → Arduino A5
VCC        → Arduino 5V
GND        → Arduino GND
```

### Speaker

```
Positive   → Arduino Pin 3
Negative   → Arduino GND
```

---

## 🚀 Quick Start (5 Steps)

### Step 1: Upload Arduino Firmware

1. Open Arduino IDE
2. Install required libraries:
   - LEDMatrixDriver
   - RTClib
   - Wire (built-in)
   - EEPROM (built-in)
   - SoftwareSerial (built-in)
3. Open `DGTclock-animated.ino`
4. Select: Tools → Board → Arduino Uno
5. Select your COM port
6. Click Upload ⬆️

### Step 2: Pair HC-05 Bluetooth

**Windows/Mac/Linux:**

1. Open Bluetooth settings
2. Search for devices
3. Select "HC-05"
4. Enter PIN: `1234` or `0000`
5. Confirm pairing

**Android:**

1. Settings → Connected devices → Pair new device
2. Select "HC-05"
3. Enter PIN: `1234`

### Step 3: Deploy Dashboard to Vercel

```bash
# Option 1: Deploy directly
cd dashboard
npx vercel

# Option 2: Run locally
cd dashboard
python3 -m http.server 8000
# Open browser: http://localhost:8000
```

### Step 4: Connect Dashboard to Clock

1. Open dashboard URL in Chrome/Edge
2. Click **"Connect to Clock"**
3. Select **"HC-05"** from device list
4. Wait for status to turn **green** ✅

### Step 5: Enjoy!

- ⏰ Click "Sync Time Now" to set correct time
- 🔔 Add alarms with custom melodies
- 💬 Send scrolling messages
- 🎵 Play music on demand
- 💡 Adjust brightness

---

## 🎯 Common Commands

### Time Sync

Click "Sync Time Now" → Arduino updates to current time

### Set Alarm

1. Click "+ Add Alarm"
2. Enter time (24-hour format)
3. Choose melody
4. Click "Save"

### Send Text

1. Type message (max 64 chars)
2. Choose Scroll or Static
3. Click "Send to Display"

### Play Music

1. Select melody from dropdown
2. Click "▶ Play"

---

## 🐛 Troubleshooting

### "Bluetooth not available"

- Use Chrome or Edge (not Firefox/Safari)
- Ensure you're on HTTPS or localhost
- Check device has Bluetooth capability

### "Device not found"

- Verify HC-05 is powered on
- Check HC-05 is paired in system settings
- Confirm HC-05 LED is blinking

### "Connection failed"

- Check wiring (RX↔TX crossover)
- Verify baud rate is 9600
- Try power cycling Arduino

### Commands not working

- Open Serial Monitor (9600 baud)
- Check for error messages
- Verify HC-05 TX/RX connections

---

## 📚 Resources

- **Full Documentation**: See `dashboard/README.md`
- **Implementation Details**: See `walkthrough.md`
- **Command Protocol**: See walkthrough for all commands

---

## ✨ Features Overview

| Feature           | Status | Description                   |
| ----------------- | ------ | ----------------------------- |
| Bluetooth Control | ✅     | Wireless connection via HC-05 |
| Time Sync         | ✅     | Auto-sync with device time    |
| Alarms            | ✅     | 3 alarms with custom melodies |
| Custom Text       | ✅     | Scroll or static display      |
| Music Player      | ✅     | 4 melodies on speaker         |
| Brightness        | ✅     | 16 levels (0-15)              |
| Auto-Reconnect    | ✅     | Reconnects on disconnect      |
| EEPROM Storage    | ✅     | Alarms persist after reboot   |

---

**Estimated Setup Time**: 15-20 minutes

**Difficulty**: Beginner-Friendly 🟢

**Support**: Check `dashboard/README.md` for detailed help

---

🎉 **Your clock is now smart!** Enjoy wireless control from any device!
