# DGT Clock Dashboard

A modern, web-based control dashboard for your Arduino LED Matrix Clock with HC-05 Bluetooth connectivity.

![Dashboard Preview](https://img.shields.io/badge/Platform-Web-blue) ![Bluetooth](https://img.shields.io/badge/Bluetooth-HC--05-green) ![License](https://img.shields.io/badge/License-MIT-yellow)

## ✨ Features

- **🔌 Bluetooth Connectivity** - Connect wirelessly to your HC-05 module using Web Bluetooth API
- **⏰ Time Synchronization** - Automatically sync clock time with your device
- **⏲️ Alarm Management** - Set up to 3 alarms with custom melodies
- **💬 Custom Text Display** - Send messages with scrolling or static display modes
- **🎵 Music Control** - Play 4 different melodies on demand
- **💡 Brightness Control** - Adjust LED matrix brightness (0-15 levels)
- **📱 Responsive Design** - Works on desktop and mobile devices
- **🎨 Premium UI** - Modern glassmorphism design with smooth animations

## 🚀 Quick Start

### Prerequisites

- **Browser**: Chrome, Edge, or Opera (Web Bluetooth support required)
- **Bluetooth**: HC-05 module must be paired with your device
- **Arduino**: DGT Clock firmware uploaded to Arduino Uno R3

### Local Development

1. **Navigate to the dashboard folder**:

   ```bash
   cd dashboard
   ```

2. **Start a local server**:

   ```bash
   python3 -m http.server 8000
   # or if you have Node.js:
   npx serve
   ```

3. **Open in browser**:
   Navigate to `http://localhost:8000`

### Deploy to Vercel

1. **Install Vercel CLI** (if not already installed):

   ```bash
   npm i -g vercel
   ```

2. **Deploy**:

   ```bash
   cd dashboard
   vercel
   ```

3. **Follow prompts** and your dashboard will be live!

Alternatively, you can connect your GitHub repo to Vercel for automatic deployments.

## 📱 Usage Guide

### Initial Setup

1. **Pair HC-05 Bluetooth Module**

   - Go to your device's Bluetooth settings
   - Pair with "HC-05" (default PIN: 1234 or 0000)

2. **Upload Arduino Firmware**

   - Open `DGTclock-animated.ino` in Arduino IDE
   - Upload to your Arduino Uno R3
   - Ensure HC-05 is connected properly (RX→Pin 4, TX→Pin 7)

3. **Open Dashboard**
   - Launch the web dashboard
   - Click "Connect to Clock"
   - Select your HC-05 device from the pairing dialog

### Features Overview

#### ⏰ Time Synchronization

- Click "Sync Time Now" to update your clock with current device time
- Time updates automatically on connection

#### ⏲️ Alarms

- Click "+ Add Alarm" to create a new alarm
- Set time (24-hour format) and choose a melody
- Toggle alarms on/off
- Delete unwanted alarms
- Maximum 3 alarms supported
- Alarms persist in EEPROM (survive power cycles)

#### 💬 Custom Text Display

- Enter text (up to 64 characters)
- Choose between **Scroll** mode (animated) or **Static** mode (centered)
- Click "Send to Display"
- Click "Show Time" to return to clock display

#### 🎵 Music Control

- Select from 4 melodies:
  - Default Melody
  - Happy Birthday
  - Alarm Sound
  - Notification
- Click "Play" to hear the melody
- Click "Stop" to silence

#### 💡 Brightness

- Adjust slider (0-15)
- Changes apply instantly
- Setting is saved locally

## 🔧 Technical Details

### Bluetooth Communication Protocol

Commands are sent in the format: `<CMD:PARAM1,PARAM2,...>`

**Supported Commands**:

- `TIME:timestamp` - Sync time (Unix timestamp)
- `ALARM:id,hh,mm,enabled,melody` - Set alarm
- `TEXT:mode,message` - Display custom text (mode: 0=scroll, 1=static)
- `MUSIC:action,melody` - Control music (action: 0=stop, 1=play)
- `BRIGHTNESS:level` - Set brightness (0-15)
- `MODE:TIME|TEXT` - Switch display mode

**Responses from Arduino**:

- `OK:MESSAGE` - Success confirmation
- `ERR:MESSAGE` - Error message
- `EVENT:ALARM_n` - Alarm triggered notification

### Browser Compatibility

| Browser | Desktop | Mobile |
| ------- | ------- | ------ |
| Chrome  | ✅      | ✅     |
| Edge    | ✅      | ✅     |
| Opera   | ✅      | ✅     |
| Firefox | ❌      | ❌     |
| Safari  | ❌      | ❌     |

_Note: Web Bluetooth API is not supported in Firefox or Safari_

### Security

- Dashboard uses HTTPS (required for Web Bluetooth)
- Bluetooth communication is encrypted at hardware level
- No personal data is transmitted to external servers
- All settings stored locally in browser localStorage

## 🛠️ Troubleshooting

### "Bluetooth not available"

- Ensure you're using a compatible browser (Chrome/Edge)
- Check that your device has Bluetooth capability
- Make sure you're serving via HTTPS or localhost

### "Device not found"

- Verify HC-05 is powered on and in pairing mode
- Ensure HC-05 is paired in your system Bluetooth settings
- Check HC-05 LED is blinking (indicates ready state)

### Connection keeps dropping

- Reduce distance between device and HC-05
- Check power supply to HC-05 (needs stable 5V)
- Verify wiring connections

### Commands not working

- Check Serial Monitor for error messages
- Verify baud rate is set to 9600 for both Serial and Bluetooth
- Ensure HC-05 RX/TX are connected correctly

## 📝 License

MIT License - feel free to modify and distribute!

## 🤝 Contributing

Contributions are welcome! Feel free to submit issues or pull requests.

## 💡 Future Enhancements

- [ ] Multiple time zones support
- [ ] Custom melody upload
- [ ] Weather display integration
- [ ] Animation presets
- [ ] Theme customization
- [ ] Mobile app (React Native)

---

Made with ❤️ for your DGT LED Matrix Clock
