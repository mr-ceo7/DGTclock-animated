// ========== STATE MANAGEMENT ==========
let bluetoothDevice = null;
let bluetoothCharacteristic = null;
let isConnected = false;
let currentAlarmEdit = null;
let alarms = [
  { id: 0, hour: null, minute: null, enabled: false, melody: 0 },
  { id: 1, hour: null, minute: null, enabled: false, melody: 0 },
  { id: 2, hour: null, minute: null, enabled: false, melody: 0 }
];

// ========== DOM ELEMENTS ==========
const elements = {
  connectBtn: document.getElementById('connectBtn'),
  statusIndicator: document.getElementById('statusIndicator'),
  statusText: document.getElementById('statusText'),
  syncTimeBtn: document.getElementById('syncTimeBtn'),
  currentTime: document.getElementById('currentTime'),
  addAlarmBtn: document.getElementById('addAlarmBtn'),
  alarmsList: document.getElementById('alarmsList'),
  customTextInput: document.getElementById('customTextInput'),
  charCount: document.getElementById('charCount'),
  sendTextBtn: document.getElementById('sendTextBtn'),
  showTimeBtn: document.getElementById('showTimeBtn'),
  melodySelect: document.getElementById('melodySelect'),
  playMusicBtn: document.getElementById('playMusicBtn'),
  stopMusicBtn: document.getElementById('stopMusicBtn'),
  brightnessSlider: document.getElementById('brightnessSlider'),
  brightnessValue: document.getElementById('brightnessValue'),
  alarmModal: document.getElementById('alarmModal'),
  saveAlarmBtn: document.getElementById('saveAlarmBtn'),
  cancelAlarmBtn: document.getElementById('cancelAlarmBtn'),
  alarmHour: document.getElementById('alarmHour'),
  alarmMinute: document.getElementById('alarmMinute'),
  alarmMelody: document.getElementById('alarmMelody')
};

// ========== INITIALIZATION ==========
document.addEventListener('DOMContentLoaded', () => {
  initEventListeners();
  updateClock();
  setInterval(updateClock, 1000);
  renderAlarms();
  loadSettings();
});

function initEventListeners() {
  elements.connectBtn.addEventListener('click', toggleConnection);
  elements.syncTimeBtn.addEventListener('click', syncTime);
  elements.addAlarmBtn.addEventListener('click', openAlarmModal);
  elements.saveAlarmBtn.addEventListener('click', saveAlarm);
  elements.cancelAlarmBtn.addEventListener('click', closeAlarmModal);
  elements.sendTextBtn.addEventListener('click', sendCustomText);
  elements.showTimeBtn.addEventListener('click', showTimeMode);
  elements.playMusicBtn.addEventListener('click', playMusic);
  elements.stopMusicBtn.addEventListener('click', stopMusic);
  elements.brightnessSlider.addEventListener('input', updateBrightnessDisplay);
  elements.brightnessSlider.addEventListener('change', setBrightness);
  elements.customTextInput.addEventListener('input', updateCharCount);
  
  // Close modal on outside click
  elements.alarmModal.addEventListener('click', (e) => {
    if (e.target === elements.alarmModal) closeAlarmModal();
  });
}

// ========== BLUETOOTH CONNECTION ==========
async function toggleConnection() {
  if (isConnected) {
    disconnect();
  } else {
    await connect();
  }
}

async function connect() {
  try {
    elements.connectBtn.disabled = true;
    elements.connectBtn.textContent = 'Connecting...';
    
    // Request Bluetooth device
    bluetoothDevice = await navigator.bluetooth.requestDevice({
      filters: [{ services: ['0000ffe0-0000-1000-8000-00805f9b34fb'] }],
      optionalServices: ['0000ffe0-0000-1000-8000-00805f9b34fb']
    });
    
    // Connect to GATT server
    const server = await bluetoothDevice.gatt.connect();
    
    // Get service
    const service = await server.getPrimaryService('0000ffe0-0000-1000-8000-00805f9b34fb');
    
    // Get characteristic
    bluetoothCharacteristic = await service.getCharacteristic('0000ffe1-0000-1000-8000-00805f9b34fb');
    
    // Listen for disconnection
    bluetoothDevice.addEventListener('gattserverdisconnected', onDisconnected);
    
    // Enable notifications
    await bluetoothCharacteristic.startNotifications();
    bluetoothCharacteristic.addEventListener('characteristicvaluechanged', handleNotification);
    
    setConnectionStatus(true);
    showNotification('Connected to DGT Clock!', 'success');
    
  } catch (error) {
    console.error('Connection failed:', error);
    showNotification('Connection failed. Make sure the device is paired and nearby.', 'error');
    setConnectionStatus(false);
  } finally {
    elements.connectBtn.disabled = false;
  }
}

function disconnect() {
  if (bluetoothDevice && bluetoothDevice.gatt.connected) {
    bluetoothDevice.gatt.disconnect();
  }
  setConnectionStatus(false);
  showNotification('Disconnected from DGT Clock', 'info');
}

function onDisconnected() {
  setConnectionStatus(false);
  showNotification('Device disconnected. Attempting to reconnect...', 'warning');
  
  // Auto-reconnect after 3 seconds
  setTimeout(() => {
    if (!isConnected) {
      connect();
    }
  }, 3000);
}

function setConnectionStatus(connected) {
  isConnected = connected;
  
  if (connected) {
    elements.statusIndicator.classList.add('connected');
    elements.statusText.textContent = 'Connected';
    elements.connectBtn.textContent = 'Disconnect';
    
    // Enable all controls
    document.querySelectorAll('.btn:not(#connectBtn)').forEach(btn => btn.disabled = false);
  } else {
    elements.statusIndicator.classList.remove('connected');
    elements.statusText.textContent = 'Disconnected';
    elements.connectBtn.textContent = 'Connect to Clock';
    
    // Disable all controls
    document.querySelectorAll('.btn:not(#connectBtn)').forEach(btn => btn.disabled = true);
  }
}

// ========== BLUETOOTH COMMUNICATION ==========
async function sendCommand(command) {
  if (!isConnected || !bluetoothCharacteristic) {
    showNotification('Not connected to device', 'error');
    return false;
  }
  
  try {
    const encoder = new TextEncoder();
    const data = encoder.encode(`<${command}>`);
    await bluetoothCharacteristic.writeValue(data);
    return true;
  } catch (error) {
    console.error('Send command failed:', error);
    showNotification('Failed to send command', 'error');
    return false;
  }
}

function handleNotification(event) {
  const decoder = new TextDecoder();
  const message = decoder.decode(event.target.value);
  console.log('Received:', message);
  
  // Handle responses from Arduino
  if (message.startsWith('OK:')) {
    showNotification(message.substring(3), 'success');
  } else if (message.startsWith('ERR:')) {
    showNotification('Error: ' + message.substring(4), 'error');
  } else if (message.startsWith('EVENT:ALARM_')) {
    const alarmId = message.substring(12);
    showNotification(`⏰ Alarm ${parseInt(alarmId) + 1} triggered!`, 'warning');
  }
}

// ========== TIME SYNCHRONIZATION ==========
function updateClock() {
  const now = new Date();
  const hours = String(now.getHours()).padStart(2, '0');
  const minutes = String(now.getMinutes()).padStart(2, '0');
  const seconds = String(now.getSeconds()).padStart(2, '0');
  elements.currentTime.textContent = `${hours}:${minutes}:${seconds}`;
}

async function syncTime() {
  const timestamp = Math.floor(Date.now() / 1000);
  const success = await sendCommand(`TIME:${timestamp}`);
  
  if (success) {
    showNotification('Time synchronized!', 'success');
  }
}

// ========== ALARM MANAGEMENT ==========
function renderAlarms() {
  const activeAlarms = alarms.filter(a => a.hour !== null);
  
  if (activeAlarms.length === 0) {
    elements.alarmsList.innerHTML = '<div class="empty-state">No alarms set. Click "Add Alarm" to create one.</div>';
    return;
  }
  
  elements.alarmsList.innerHTML = activeAlarms.map(alarm => {
    const melodyNames = ['Default', 'Birthday', 'Alarm', 'Notification'];
    return `
      <div class="alarm-item">
        <div class="alarm-info">
          <div class="alarm-time">${String(alarm.hour).padStart(2, '0')}:${String(alarm.minute).padStart(2, '0')}</div>
          <div class="alarm-melody">🎵 ${melodyNames[alarm.melody]}</div>
        </div>
        <div class="alarm-actions">
          <div class="alarm-toggle ${alarm.enabled ? 'active' : ''}" onclick="toggleAlarm(${alarm.id})"></div>
          <button class="alarm-delete" onclick="deleteAlarm(${alarm.id})">Delete</button>
        </div>
      </div>
    `;
  }).join('');
  
  saveSettings();
}

function openAlarmModal(alarmId = null) {
  currentAlarmEdit = alarmId;
  
  if (alarmId !== null) {
    const alarm = alarms[alarmId];
    elements.alarmHour.value = alarm.hour || '';
    elements.alarmMinute.value = alarm.minute || '';
    elements.alarmMelody.value = alarm.melody;
  } else {
    // Find first empty slot
    const emptySlot = alarms.find(a => a.hour === null);
    if (!emptySlot) {
      showNotification('Maximum 3 alarms allowed', 'warning');
      return;
    }
    currentAlarmEdit = emptySlot.id;
    elements.alarmHour.value = '';
    elements.alarmMinute.value = '';
    elements.alarmMelody.value = '0';
  }
  
  elements.alarmModal.classList.add('active');
}

function closeAlarmModal() {
  elements.alarmModal.classList.remove('active');
  currentAlarmEdit = null;
}

async function saveAlarm() {
  const hour = parseInt(elements.alarmHour.value);
  const minute = parseInt(elements.alarmMinute.value);
  const melody = parseInt(elements.alarmMelody.value);
  
  if (isNaN(hour) || isNaN(minute) || hour < 0 || hour > 23 || minute < 0 || minute > 59) {
    showNotification('Invalid time format', 'error');
    return;
  }
  
  alarms[currentAlarmEdit] = {
    id: currentAlarmEdit,
    hour,
    minute,
    enabled: true,
    melody
  };
  
  const command = `ALARM:${currentAlarmEdit},${String(hour).padStart(2, '0')},${String(minute).padStart(2, '0')},1,${melody}`;
  const success = await sendCommand(command);
  
  if (success) {
    renderAlarms();
    closeAlarmModal();
  }
}

async function toggleAlarm(id) {
  alarms[id].enabled = !alarms[id].enabled;
  const alarm = alarms[id];
  
  const command = `ALARM:${id},${String(alarm.hour).padStart(2, '0')},${String(alarm.minute).padStart(2, '0')},${alarm.enabled ? 1 : 0},${alarm.melody}`;
  const success = await sendCommand(command);
  
  if (success) {
    renderAlarms();
  }
}

async function deleteAlarm(id) {
  if (!confirm('Delete this alarm?')) return;
  
  alarms[id] = {
    id,
    hour: null,
    minute: null,
    enabled: false,
    melody: 0
  };
  
  const command = `ALARM:${id},00,00,0,0`;
  await sendCommand(command);
  renderAlarms();
}

// Make functions global for inline onclick handlers
window.toggleAlarm = toggleAlarm;
window.deleteAlarm = deleteAlarm;

// ========== CUSTOM TEXT ==========
function updateCharCount() {
  const length = elements.customTextInput.value.length;
  elements.charCount.textContent = length;
}

async function sendCustomText() {
  const text = elements.customTextInput.value.trim();
  
  if (!text) {
    showNotification('Please enter some text', 'warning');
    return;
  }
  
  const mode = document.querySelector('input[name="textMode"]:checked').value;
  const modeNum = mode === 'scroll' ? 0 : 1;
  
  const command = `TEXT:${modeNum},${text}`;
  const success = await sendCommand(command);
  
  if (success) {
    elements.customTextInput.value = '';
    updateCharCount();
  }
}

async function showTimeMode() {
  await sendCommand('MODE:TIME');
}

// ========== MUSIC CONTROL ==========
async function playMusic() {
  const melody = elements.melodySelect.value;
  await sendCommand(`MUSIC:1,${melody}`);
}

async function stopMusic() {
  await sendCommand('MUSIC:0,0');
}

// ========== BRIGHTNESS CONTROL ==========
function updateBrightnessDisplay() {
  elements.brightnessValue.textContent = elements.brightnessSlider.value;
}

async function setBrightness() {
  const level = elements.brightnessSlider.value;
  await sendCommand(`BRIGHTNESS:${level}`);
  saveSettings();
}

// ========== SETTINGS PERSISTENCE ==========
function saveSettings() {
  localStorage.setItem('dgtClockAlarms', JSON.stringify(alarms));
  localStorage.setItem('dgtClockBrightness', elements.brightnessSlider.value);
}

function loadSettings() {
  const savedAlarms = localStorage.getItem('dgtClockAlarms');
  if (savedAlarms) {
    alarms = JSON.parse(savedAlarms);
    renderAlarms();
  }
  
  const savedBrightness = localStorage.getItem('dgtClockBrightness');
  if (savedBrightness) {
    elements.brightnessSlider.value = savedBrightness;
    updateBrightnessDisplay();
  }
}

// ========== NOTIFICATIONS ==========
function showNotification(message, type = 'info') {
  // Create notification element
  const notification = document.createElement('div');
  notification.className = 'notification';
  notification.style.cssText = `
    position: fixed;
    bottom: 24px;
    right: 24px;
    padding: 16px 24px;
    background: ${type === 'success' ? 'rgba(16, 185, 129, 0.2)' : 
                  type === 'error' ? 'rgba(239, 68, 68, 0.2)' :
                  type === 'warning' ? 'rgba(245, 158, 11, 0.2)' :
                  'rgba(99, 102, 241, 0.2)'};
    border: 1px solid ${type === 'success' ? '#10b981' : 
                        type === 'error' ? '#ef4444' :
                        type === 'warning' ? '#f59e0b' :
                        '#6366f1'};
    border-radius: 12px;
    color: white;
    font-size: 14px;
    font-weight: 500;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.5);
    z-index: 10000;
    animation: slideIn 0.3s ease-out;
    max-width: 300px;
  `;
  
  notification.textContent = message;
  document.body.appendChild(notification);
  
  // Auto remove after 3 seconds
  setTimeout(() => {
    notification.style.animation = 'slideOut 0.3s ease-out';
    setTimeout(() => notification.remove(), 300);
  }, 3000);
}

// Add notification animations to CSS
const style = document.createElement('style');
style.textContent = `
  @keyframes slideIn {
    from {
      opacity: 0;
      transform: translateX(100px);
    }
    to {
      opacity: 1;
      transform: translateX(0);
    }
  }
  
  @keyframes slideOut {
    from {
      opacity: 1;
      transform: translateX(0);
    }
    to {
      opacity: 0;
      transform: translateX(100px);
    }
  }
`;
document.head.appendChild(style);

// ========== INITIALIZATION MESSAGE ==========
console.log('🕐 DGT Clock Dashboard loaded successfully!');
console.log('Make sure your HC-05 module is paired with this device before connecting.');
