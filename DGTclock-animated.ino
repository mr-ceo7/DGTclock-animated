#include <LEDMatrixDriver.hpp>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "pitches.h"

// ========== BLUETOOTH CONFIGURATION ==========
#define BT_RX 4  // Arduino RX connected to HC-05 TX
#define BT_TX 7  // Arduino TX connected to HC-05 RX
SoftwareSerial bluetooth(BT_RX, BT_TX);

// ========== LED MATRIX CONFIGURATION ==========
const uint8_t LEDMATRIX_CS_PIN = 9;
const int LEDMATRIX_SEGMENTS = 4;
const int LEDMATRIX_WIDTH = LEDMATRIX_SEGMENTS * 8;

LEDMatrixDriver lmd(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);
RTC_DS3231 rtc;

// ========== MELODY DEFINITIONS ==========
// Melody 0: Default melody
int melody0[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
int melody0Durations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

// Melody 1: Happy Birthday
int melody1[] = {
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,
  NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4
};
int melody1Durations[] = {
  8, 8, 4, 4, 4, 2,
  8, 8, 4, 4, 4, 2
};

// Melody 2: Alarm
int melody2[] = {
  NOTE_E5, NOTE_E5, 0, NOTE_E5, NOTE_E5, 0, NOTE_E5, NOTE_E5
};
int melody2Durations[] = {
  8, 8, 8, 8, 8, 8, 8, 8
};

// Melody 3: Notification
int melody3[] = {
  NOTE_C5, NOTE_E5, NOTE_G5
};
int melody3Durations[] = {
  8, 8, 4
};

// ========== ALARM STRUCTURE ==========
struct Alarm {
  uint8_t hour;
  uint8_t minute;
  bool enabled;
  uint8_t melodyIndex;
};

const int MAX_ALARMS = 3;
const int EEPROM_ALARM_START = 0;
const int ALARM_SIZE = 4; // hour(1) + minute(1) + enabled(1) + melody(1)

Alarm alarms[MAX_ALARMS];

// ========== DISPLAY STATE ==========
enum DisplayMode {
  MODE_TIME,
  MODE_CUSTOM_TEXT
};

DisplayMode currentMode = MODE_TIME;
char customText[65] = "";
bool scrollMode = true;
int customTextPos = 0;

// ========== BLUETOOTH BUFFER ==========
String btCommand = "";

// ========== TIME TRACKING ==========
char timeBuffer[6];  // HHMM + null terminator
int prevHour = -1;
int prevMinute = -1;

// ========== FONT DEFINITION ==========
byte font[95][8] = {
  {0,0,0,0,0,0,0,0}, // SPACE
  {0x10,0x18,0x18,0x18,0x18,0x00,0x18,0x18}, // !
  {0x28,0x28,0x08,0x00,0x00,0x00,0x00,0x00}, // "
  {0x00,0x0a,0x7f,0x14,0x28,0xfe,0x50,0x00}, // #
  {0x10,0x38,0x54,0x70,0x1c,0x54,0x38,0x10}, // $
  {0x00,0x60,0x66,0x08,0x10,0x66,0x06,0x00}, // %
  {0,0,0,0,0,0,0,0}, // &
  {0x00,0x10,0x18,0x18,0x08,0x00,0x00,0x00}, // '
  {0x02,0x04,0x08,0x08,0x08,0x08,0x08,0x04}, // (
  {0x40,0x20,0x10,0x10,0x10,0x10,0x10,0x20}, // )
  {0x00,0x10,0x54,0x38,0x10,0x38,0x54,0x10}, // *
  {0x00,0x08,0x08,0x08,0x7f,0x08,0x08,0x08}, // +
  {0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x08}, // ,
  {0x00,0x00,0x00,0x00,0x7e,0x00,0x00,0x00}, // -
  {0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x06}, // .
  {0x00,0x04,0x04,0x08,0x10,0x20,0x40,0x40}, // /
  {0x00,0x38,0x44,0x4c,0x54,0x64,0x44,0x38}, // 0
  {0x04,0x0c,0x14,0x24,0x04,0x04,0x04,0x04}, // 1
  {0x00,0x30,0x48,0x04,0x04,0x38,0x40,0x7c}, // 2
  {0x00,0x38,0x04,0x04,0x18,0x04,0x44,0x38}, // 3
  {0x00,0x04,0x0c,0x14,0x24,0x7e,0x04,0x04}, // 4
  {0x00,0x7c,0x40,0x40,0x78,0x04,0x04,0x38}, // 5
  {0x00,0x38,0x40,0x40,0x78,0x44,0x44,0x38}, // 6
  {0x00,0x7c,0x04,0x04,0x08,0x08,0x10,0x10}, // 7
  {0x00,0x3c,0x44,0x44,0x38,0x44,0x44,0x78}, // 8
  {0x00,0x38,0x44,0x44,0x3c,0x04,0x04,0x78}, // 9
  {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x00}, // :
  {0x00,0x18,0x18,0x00,0x00,0x18,0x18,0x08}, // ;
  {0x00,0x10,0x20,0x40,0x80,0x40,0x20,0x10}, // <
  {0x00,0x00,0x7e,0x00,0x00,0xfc,0x00,0x00}, // =
  {0x00,0x08,0x04,0x02,0x01,0x02,0x04,0x08}, // >
  {0x00,0x38,0x44,0x04,0x08,0x10,0x00,0x10}, // ?
  {0x00,0x30,0x48,0xba,0xba,0x84,0x78,0x00}, // @
  {0x00,0x1c,0x22,0x42,0x42,0x7e,0x42,0x42}, // A
  {0x00,0x78,0x44,0x44,0x78,0x44,0x44,0x7c}, // B
  {0x00,0x3c,0x44,0x40,0x40,0x40,0x44,0x7c}, // C
  {0x00,0x7c,0x42,0x42,0x42,0x42,0x44,0x78}, // D
  {0x00,0x78,0x40,0x40,0x70,0x40,0x40,0x7c}, // E
  {0x00,0x7c,0x40,0x40,0x78,0x40,0x40,0x40}, // F
  {0x00,0x3c,0x40,0x40,0x5c,0x44,0x44,0x78}, // G
  {0x00,0x42,0x42,0x42,0x7e,0x42,0x42,0x42}, // H
  {0x00,0x7c,0x10,0x10,0x10,0x10,0x10,0x7e}, // I
  {0x00,0x7e,0x02,0x02,0x02,0x02,0x04,0x38}, // J
  {0x00,0x44,0x48,0x50,0x60,0x50,0x48,0x44}, // K
  {0x00,0x40,0x40,0x40,0x40,0x40,0x40,0x7c}, // L
  {0x00,0x82,0xc6,0xaa,0x92,0x82,0x82,0x82}, // M
  {0x00,0x42,0x42,0x62,0x52,0x4a,0x46,0x42}, // N
  {0x00,0x3c,0x42,0x42,0x42,0x42,0x44,0x38}, // O
  {0x00,0x78,0x44,0x44,0x48,0x70,0x40,0x40}, // P
  {0x00,0x3c,0x42,0x42,0x52,0x4a,0x44,0x3a}, // Q
  {0x00,0x78,0x44,0x44,0x78,0x50,0x48,0x44}, // R
  {0x00,0x38,0x40,0x40,0x38,0x04,0x04,0x78}, // S
  {0x00,0x7e,0x90,0x10,0x10,0x10,0x10,0x10}, // T
  {0x00,0x42,0x42,0x42,0x42,0x42,0x42,0x3e}, // U
  {0x00,0x42,0x42,0x42,0x42,0x44,0x28,0x10}, // V
  {0x80,0x82,0x82,0x92,0x92,0x92,0x94,0x78}, // W
  {0x00,0x42,0x42,0x24,0x18,0x24,0x42,0x42}, // X
  {0x00,0x44,0x44,0x28,0x10,0x10,0x10,0x10}, // Y
  {0x00,0x7c,0x04,0x08,0x7c,0x20,0x40,0xfe}, // Z
};

// ========== SETUP ==========
void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  Wire.begin();
  rtc.begin();
  
  // Initialize LED matrix
  lmd.setEnabled(true);
  lmd.setIntensity(8);
  
  // Load alarms from EEPROM
  loadAlarms();
  
  bluetooth.println("DGT_Clock Ready");
}

// ========== MAIN LOOP ==========
void loop() {
  // Handle Bluetooth commands
  handleBluetooth();
  
  // Check alarms
  checkAlarms();
  
  // Update display based on mode
  if (currentMode == MODE_TIME) {
    updateTimeDisplay();
  } else {
    updateCustomTextDisplay();
  }
  
  delay(100);
}

// ========== BLUETOOTH HANDLING ==========
void handleBluetooth() {
  while (bluetooth.available()) {
    char c = bluetooth.read();
    
    if (c == '<') {
      btCommand = "";
    } else if (c == '>') {
      processCommand(btCommand);
      btCommand = "";
    } else {
      btCommand += c;
    }
  }
}

void processCommand(String cmd) {
  int colonPos = cmd.indexOf(':');
  if (colonPos == -1) return;
  
  String command = cmd.substring(0, colonPos);
  String params = cmd.substring(colonPos + 1);
  
  if (command == "TIME") {
    syncTime(params);
  } else if (command == "ALARM") {
    setAlarm(params);
  } else if (command == "TEXT") {
    setCustomText(params);
  } else if (command == "MUSIC") {
    playMusic(params);
  } else if (command == "BRIGHTNESS") {
    setBrightness(params.toInt());
  } else if (command == "MODE") {
    setDisplayMode(params);
  }
}

// ========== TIME SYNCHRONIZATION ==========
void syncTime(String timestamp) {
  unsigned long ts = timestamp.toInt();
  
  if (ts > 0) {
    DateTime newTime(ts);
    rtc.adjust(newTime);
    bluetooth.println("OK:TIME_SYNCED");
    playNotification();
  } else {
    bluetooth.println("ERR:INVALID_TIME");
  }
}

// ========== ALARM MANAGEMENT ==========
void setAlarm(String params) {
  // Format: ID,HH,MM,ENABLED,MELODY
  int idx = params.indexOf(',');
  int alarmId = params.substring(0, idx).toInt();
  params = params.substring(idx + 1);
  
  if (alarmId < 0 || alarmId >= MAX_ALARMS) {
    bluetooth.println("ERR:INVALID_ALARM_ID");
    return;
  }
  
  idx = params.indexOf(',');
  int hour = params.substring(0, idx).toInt();
  params = params.substring(idx + 1);
  
  idx = params.indexOf(',');
  int minute = params.substring(0, idx).toInt();
  params = params.substring(idx + 1);
  
  idx = params.indexOf(',');
  bool enabled = params.substring(0, idx).toInt() == 1;
  params = params.substring(idx + 1);
  
  int melody = params.toInt();
  
  alarms[alarmId].hour = hour;
  alarms[alarmId].minute = minute;
  alarms[alarmId].enabled = enabled;
  alarms[alarmId].melodyIndex = melody;
  
  saveAlarms();
  bluetooth.println("OK:ALARM_SET");
  playNotification();
}

void checkAlarms() {
  DateTime now = rtc.now();
  
  for (int i = 0; i < MAX_ALARMS; i++) {
    if (alarms[i].enabled && 
        alarms[i].hour == now.hour() && 
        alarms[i].minute == now.minute() &&
        now.second() == 0) {  // Trigger only once per minute
      
      triggerAlarm(i);
    }
  }
}

void triggerAlarm(int alarmId) {
  bluetooth.print("EVENT:ALARM_");
  bluetooth.println(alarmId);
  
  // Play alarm melody
  playMelodyByIndex(alarms[alarmId].melodyIndex);
  
  // Flash display
  for (int i = 0; i < 3; i++) {
    lmd.setEnabled(false);
    delay(300);
    lmd.setEnabled(true);
    delay(300);
  }
}

// ========== CUSTOM TEXT DISPLAY ==========
void setCustomText(String params) {
  // Format: MODE,TEXT
  int idx = params.indexOf(',');
  int mode = params.substring(0, idx).toInt();
  String text = params.substring(idx + 1);
  
  scrollMode = (mode == 0);
  text.toCharArray(customText, 65);
  customTextPos = scrollMode ? LEDMATRIX_WIDTH : 0;
  currentMode = MODE_CUSTOM_TEXT;
  
  bluetooth.println("OK:TEXT_SET");
  playNotification();
}

void updateCustomTextDisplay() {
  lmd.clear();
  
  if (scrollMode) {
    drawString(customText, strlen(customText), customTextPos, 0);
    lmd.display();
    
    customTextPos--;
    if (customTextPos < -((int)strlen(customText) * 8)) {
      customTextPos = LEDMATRIX_WIDTH;
    }
    delay(40);
  } else {
    // Static mode - center text
    int textWidth = strlen(customText) * 8;
    int startX = (LEDMATRIX_WIDTH - textWidth) / 2;
    drawString(customText, strlen(customText), startX, 0);
    lmd.display();
    delay(500);
  }
}

// ========== MUSIC CONTROL ==========
void playMusic(String params) {
  // Format: PLAY/STOP,MELODY_INDEX
  int idx = params.indexOf(',');
  int action = params.substring(0, idx).toInt();
  int melodyIdx = params.substring(idx + 1).toInt();
  
  if (action == 1) {
    playMelodyByIndex(melodyIdx);
    bluetooth.println("OK:MUSIC_PLAYED");
  } else {
    noTone(3);
    bluetooth.println("OK:MUSIC_STOPPED");
  }
}

void playMelodyByIndex(int idx) {
  int* melody;
  int* durations;
  int melodyLength;
  
  switch(idx) {
    case 0:
      melody = melody0;
      durations = melody0Durations;
      melodyLength = 8;
      break;
    case 1:
      melody = melody1;
      durations = melody1Durations;
      melodyLength = 12;
      break;
    case 2:
      melody = melody2;
      durations = melody2Durations;
      melodyLength = 8;
      break;
    case 3:
      melody = melody3;
      durations = melody3Durations;
      melodyLength = 3;
      break;
    default:
      return;
  }
  
  for (int i = 0; i < melodyLength; i++) {
    int noteDuration = 1000 / durations[i];
    tone(3, melody[i], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(3);
  }
}

void playNotification() {
  tone(3, NOTE_C5, 100);
  delay(120);
  noTone(3);
}

// ========== DISPLAY MODE ==========
void setDisplayMode(String mode) {
  if (mode == "TIME") {
    currentMode = MODE_TIME;
    bluetooth.println("OK:MODE_TIME");
  } else if (mode == "TEXT") {
    currentMode = MODE_CUSTOM_TEXT;
    bluetooth.println("OK:MODE_TEXT");
  }
}

// ========== BRIGHTNESS CONTROL ==========
void setBrightness(int level) {
  if (level >= 0 && level <= 15) {
    lmd.setIntensity(level);
    bluetooth.println("OK:BRIGHTNESS_SET");
  } else {
    bluetooth.println("ERR:INVALID_BRIGHTNESS");
  }
}

// ========== TIME DISPLAY ==========
void updateTimeDisplay() {
  DateTime now = rtc.now();
  int hour12 = now.hour() % 12;
  if (hour12 == 0) hour12 = 12;
  
  if (now.minute() != prevMinute || now.hour() != prevHour) {
    prevMinute = now.minute();
    prevHour = now.hour();
    
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d%02d", hour12, now.minute());
    animateTimeChange();
  }
  
  int len = strlen(timeBuffer);
  lmd.clear();
  drawString(timeBuffer, len, 0, 0);
  lmd.display();
  delay(500);
}

// ========== ANIMATION ==========
void animateTimeChange() {
  playMelodyByIndex(0);
  
  int len = strlen(timeBuffer);
  int startX = LEDMATRIX_WIDTH;
  reverseText(timeBuffer);
  
  for (int pos = startX; pos >= 0; pos--) {
    lmd.clear();
    drawString(timeBuffer, len, pos, 0);
    lmd.display();
    delay(40);
  }
}

// ========== DRAWING FUNCTIONS ==========
void drawString(char* text, int len, int x, int y) {
  for (int idx = 0; idx < len; idx++) {
    int c = text[idx] - 32;
    
    if (c < 0 || c >= 95) continue;
    
    if (x + idx * 8 > LEDMATRIX_WIDTH)
      return;
    
    if (8 + x + idx * 8 > 0)
      drawSprite(font[c], x + idx * 8, y, 8, 8);
  }
}

void drawSprite(byte* sprite, int x, int y, int width, int height) {
  byte mask = B10000000;
  
  for (int iy = 0; iy < height; iy++) {
    for (int ix = 0; ix < width; ix++) {
      lmd.setPixel(x + ix, y + iy, (bool)(sprite[iy] & mask));
      mask >>= 1;
    }
    mask = B10000000;
  }
}

void reverseText(char* str) {
  int len = strlen(str);
  for (int i = 0; i < len / 2; i++) {
    char temp = str[i];
    str[i] = str[len - 1 - i];
    str[len - 1 - i] = temp;
  }
}

// ========== EEPROM FUNCTIONS ==========
void saveAlarms() {
  for (int i = 0; i < MAX_ALARMS; i++) {
    int addr = EEPROM_ALARM_START + (i * ALARM_SIZE);
    EEPROM.write(addr, alarms[i].hour);
    EEPROM.write(addr + 1, alarms[i].minute);
    EEPROM.write(addr + 2, alarms[i].enabled ? 1 : 0);
    EEPROM.write(addr + 3, alarms[i].melodyIndex);
  }
}

void loadAlarms() {
  for (int i = 0; i < MAX_ALARMS; i++) {
    int addr = EEPROM_ALARM_START + (i * ALARM_SIZE);
    alarms[i].hour = EEPROM.read(addr);
    alarms[i].minute = EEPROM.read(addr + 1);
    alarms[i].enabled = EEPROM.read(addr + 2) == 1;
    alarms[i].melodyIndex = EEPROM.read(addr + 3);
    
    // Validate loaded data
    if (alarms[i].hour > 23) alarms[i].hour = 0;
    if (alarms[i].minute > 59) alarms[i].minute = 0;
    if (alarms[i].melodyIndex > 3) alarms[i].melodyIndex = 0;
  }
}
