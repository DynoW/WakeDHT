const DEV_MODE = false; // Set to true for local development so you can see temperature/humidity changes

// Define the computers
const computers = [
  { name: 'device1', mac: 'AA-BB-CC-DD-EE-00', ip: '192.168.1.100', port: 22 },
  { name: 'device2', mac: 'AA-BB-CC-DD-EE-01', ip: '192.168.1.101', port: 22 }
];

// Check the local storage for theme preference on page load
if (localStorage.getItem('theme') === 'dark' ||
  (!localStorage.getItem('theme') && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
  document.body.classList.add('dark');
} else {
  document.body.classList.remove('dark');
}

// Wait for DOM to be fully loaded before adding event listeners
document.addEventListener('DOMContentLoaded', () => {
  // Get the theme toggle button
  const themeToggle = document.getElementById('theme-toggle');
  
  // Toggle the theme when the button is clicked
  themeToggle.addEventListener('click', () => {
    document.body.classList.toggle('dark');

    // Save the theme preference to local storage
    if (document.body.classList.contains('dark')) {
      localStorage.setItem('theme', 'dark');
    } else {
      localStorage.setItem('theme', 'light');
    }
  });
});

// Function to update the progress of the temperature and humidity circles
function updateProgress(value, id) {
  // Get the circle and text elements by their IDs
  const circle = document.getElementById(id);
  const text = document.getElementById(id + '-text');

  if (!circle || !text) return; // Safety check

  // Calculate the circumference of the circle
  const radius = circle.r.baseVal.value;
  const circumference = 2 * Math.PI * radius;
  let offset;

  // Determine the offset based on the value and id
  if (value < 0) {
    offset = circumference;
  } else if ((id === 't' && value > 40) || (id === 'h' && value > 100)) {
    offset = 0;
  } else {
    offset = circumference - (value / (id === 't' ? 40 : 100)) * circumference;
  }

  // Set the stroke dash offset to create the progress effect
  circle.style.strokeDashoffset = offset;

  // Define the conditions for temperature and humidity
  const conditions = {
    t: [
      { max: 18, color: '#3b82f6', label: '(Cold)' },
      { max: 24, color: '#22c55e', label: '(Comfortable)' },
      { max: 28, color: '#eab308', label: '(Warm)' },
      { max: 32, color: '#f97316', label: '(Hot)' },
      { max: Infinity, color: '#ef4444', label: '(Very Hot)' }
    ],
    h: [
      { max: 30, color: '#3b82f6', label: '(Dry)' },
      { max: 60, color: '#22c55e', label: '(Comfortable)' },
      { max: 70, color: '#eab308', label: '(Humid)' },
      { max: 80, color: '#f97316', label: '(Very Humid)' },
      { max: Infinity, color: '#ef4444', label: '(Excessive)' }
    ]
  };

  // Find the appropriate condition based on the value
  const condition = conditions[id].find(cond => (value <= cond.max));

  // // Remove all stroke-related CSS classes
  // Array.from(circle.classList).forEach(cls => {
  //   if (cls.startsWith('stroke-')) {
  //     circle.classList.remove(cls);
  //   }
  // });

  // Set stroke color directly via style for better compatibility
  circle.style.stroke = condition.color;

  // Update the text content with the value and condition label
  text.innerHTML = value + (id === 't' ? '&#8451;' : '&#37') + '<br>' + condition.label;
}

// Get the status text by their element ID
statusText = document.getElementById('status');

// Simplified fetch with better board status messages
let isLoadingSensorData = false;
let consecutiveFailures = 0;
const MAX_FAILURES = 3;
let sensorUpdateInterval = null;
let reconnectCountdown = 0;
let countdownInterval = null;
let isReconnecting = false;

const baseUrl = DEV_MODE ? 'http://esp32.local' : '';

function getTempAndHumidity() {
  if (isLoadingSensorData) return;
  
  isLoadingSensorData = true;
  
  fetch(`${baseUrl}/api`, {
      method: 'GET',
      headers: { 'Cache-Control': 'no-cache' },
      signal: AbortSignal.timeout(2000)
    })
    .then(response => {
      if (!response.ok) {
        return Promise.reject({ nonRetry: true, message: `HTTP ${response.status}` });
      }
      return response.json();
    })
    .then(data => {
      if (data?.valid === false) {
        updateProgress(data.temperature || 0, 't');
        updateProgress(data.humidity || 0, 'h');
        statusText.innerHTML = 'Connected';
        statusText.style.color = 'orange';
        consecutiveFailures = 0;
        // Successfully connected (even with invalid sensor data) — stop reconnect flow
        isReconnecting = false;
        clearCountdown();
        
        // Restart sensor updates after successful connection
        if (!sensorUpdateInterval) startSensorUpdates();
        return;
      }

      updateProgress(data.temperature, 't');
      updateProgress(data.humidity, 'h');
      statusText.innerHTML = 'Connected';
      statusText.style.color = 'green';
      consecutiveFailures = 0;
      // Successfully connected — stop any reconnect flow
      isReconnecting = false;
      clearCountdown();

      // Always restart sensor updates after successful connection
      if (!sensorUpdateInterval) startSensorUpdates();
    })
    .catch(error => {
      if (error?.nonRetry) return;

      // If we are already in a reconnect attempt, restart the countdown immediately
      if (isReconnecting) {
        console.warn('Reconnection attempt failed, restarting countdown...');
        startReconnectionProcess();
        return;
      }

      consecutiveFailures++;
      if (consecutiveFailures >= MAX_FAILURES) {
        startReconnectionProcess();
      }
    })
    .finally(() => {
      isLoadingSensorData = false;
    });
}

function clearCountdown() {
  if (countdownInterval) {
    clearInterval(countdownInterval);
    countdownInterval = null;
  }
}

function startReconnectionProcess() {
  // If already reconnecting, clear existing countdown and restart it
  if (isReconnecting) {
    clearCountdown();
  }
  isReconnecting = true;

  // Always clear any existing countdown first
  stopSensorUpdates();

  reconnectCountdown = 8;
  statusText.innerHTML = `Reconnecting in ${reconnectCountdown} seconds`;
  statusText.style.color = 'red';

  countdownInterval = setInterval(() => {
    reconnectCountdown--;
    if (reconnectCountdown > 1) {
      statusText.innerHTML = `Reconnecting in ${reconnectCountdown} seconds`;
    } else if (reconnectCountdown === 1) {
      statusText.innerHTML = 'Reconnecting...';
      statusText.style.color = 'orange';
    } else {
      // Countdown finished, attempt reconnection
      clearCountdown();

      // Reset failures and try again
      consecutiveFailures = 0;

      // Use a timeout to ensure the status message is visible briefly
      setTimeout(() => {
        // Make a single attempt. If it fails, getTempAndHumidity's catch will
        // detect isReconnecting and restart the countdown by calling
        // startReconnectionProcess() again.
        getTempAndHumidity();
      }, 500);
    }
  }, 1000);
}

function startSensorUpdates() {
  if (sensorUpdateInterval) return;
  
  sensorUpdateInterval = setInterval(() => {
    if (consecutiveFailures < MAX_FAILURES) {
      getTempAndHumidity();
    }
  }, 2500); // Optimized to 2.5 seconds for better performance
}

function stopSensorUpdates() {
  if (sensorUpdateInterval) {
    clearInterval(sensorUpdateInterval);
    sensorUpdateInterval = null;
  }
}

// Start sensor updates
startSensorUpdates();
getTempAndHumidity();

// Test the progress update function in while development
// This function generates random values for temperature and humidity every 3 seconds to simulate the api
// Uncomment the following lines to test the progress update function
// function random(min, max) {
//   return Math.floor(Math.random() * (max - min + 1)) + min;
// }
// setInterval(() => {
//   updateProgress(random(24, 30), 't');
//   updateProgress(random(50, 60), 'h');
//   statusText.innerHTML = 'Connected';
//   statusText.style.color = 'green';
// }, 3000);

// Computer management section

// Add a sanitized id for each computer to use in element IDs (prevents issues with spaces/symbols)
computers.forEach(c => {
  // Replace any character not allowed in HTML id with underscore
  c._id = c.name.replace(/[^A-Za-z0-9\-_:./]/g, '_');
});

// Function to create computer divs dynamically with refresh button
function createComputerDivs() {
  const container = document.getElementById('computer-container');
  // Create computer entries
  computers.forEach((computer, index) => {
    const computerDiv = document.createElement('div');
    const isLast = index === computers.length - 1;
    computerDiv.className = isLast ? '' : 'border-b';
    
    computerDiv.innerHTML = `
      <div class="grid md:grid-cols-6 grid-cols-1 p-3 gap-3 items-center">
        <div class="px-4 font-medium md:font-normal md:col-span-2">
          <span class="md:hidden font-bold mr-2">Name:</span>${computer.name}
        </div>
        <div class="px-4 md:col-span-1 min-w-20">
          <span class="md:hidden font-bold mr-2">Status:</span><span id="status-${computer._id}" class="text-gray-600">Unknown</span>
        </div>
        <div class="px-4 flex justify-center gap-2 md:col-span-3">
          <button id="wol-${computer._id}" class="bg-green-500 hover:bg-green-600 text-white p-2 rounded cursor-pointer text-base">
            Power on
          </button>
          <button id="refresh-${computer._id}" class="bg-blue-500 hover:bg-blue-600 text-white p-2 rounded cursor-pointer text-base flex items-center justify-center" title="Check status">
            <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
              <path d="M3 12a9 9 0 0 1 9-9 9.75 9.75 0 0 1 6.74 2.74L21 8"/>
              <path d="M21 3v5h-5"/>
              <path d="M21 12a9 9 0 0 1-9 9 9.75 9.75 0 0 1-6.74-2.74L3 16"/>
              <path d="M3 21v-5h5"/>
            </svg>
          </button>
        </div>
      </div>
    `;
    
    container.appendChild(computerDiv);
  });
}

// Manual computer status checking - no automatic intervals
let isCheckingStatus = false;

async function checkSingleComputerStatus(computerId) {
  const computer = computers.find(c => c._id === computerId);
  if (!computer) return;

  const statusElement = document.getElementById(`status-${computerId}`);
  const refreshButton = document.getElementById(`refresh-${computerId}`);
  
  if (!statusElement || !refreshButton) return;

  // Show loading state
  const originalRefreshText = refreshButton.innerHTML;
  refreshButton.innerHTML = '<svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" class="animate-spin"><circle cx="12" cy="12" r="10"/><path d="M12 6v6l4 2"/></svg>';
  refreshButton.disabled = true;

  statusElement.textContent = 'Checking...';
  statusElement.className = 'text-yellow-600';

  try {
    const response = await fetch(`${baseUrl}/ping?ip=${computer.ip}&port=${computer.port}`, {
      method: 'GET',
      headers: { 'Cache-Control': 'no-cache' },
      signal: AbortSignal.timeout(4000) // Increased timeout to match ESP32 timeout
    });
    
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    
    const data = await response.json();
    statusElement.textContent = data.online ? 'Online' : 'Offline';
    statusElement.className = data.online ? 'text-green-600' : 'text-red-600';
  } catch (error) {
    // Show Unknown for network errors, timeouts, or no response
    statusElement.textContent = 'Unknown';
    statusElement.className = 'text-gray-600';
    
    if (error.name === 'AbortError' || error.message.includes('timeout')) {
      console.warn(`Ping timeout: ${computer.name}`);
    }
  } finally {
    refreshButton.innerHTML = originalRefreshText;
    refreshButton.disabled = false;
  }
}

// Initial load: check computers one by one with delays to avoid binding conflicts
async function checkComputersOnLoad() {
  if (isCheckingStatus) return;
  
  isCheckingStatus = true;
  
  // Check computers sequentially with delays to prevent NS_Binding_Aborted
  for (let i = 0; i < computers.length; i++) {
    const computer = computers[i];
    const statusElement = document.getElementById(`status-${computer._id}`);
    
    if (statusElement) {
      statusElement.textContent = 'Checking...';
      statusElement.className = 'text-yellow-600';
      
      try {
        const response = await fetch(`${baseUrl}/ping?ip=${computer.ip}&port=${computer.port}`, {
          method: 'GET',
          headers: { 'Cache-Control': 'no-cache' },
          signal: AbortSignal.timeout(5000) // Increased timeout for initial load
        });
        
        if (response.ok) {
          const data = await response.json();
          statusElement.textContent = data.online ? 'Online' : 'Offline';
          statusElement.className = data.online ? 'text-green-600' : 'text-red-600';
        } else {
          // No valid response - show as Unknown
          statusElement.textContent = 'Unknown';
          statusElement.className = 'text-gray-600';
        }
      } catch (error) {
        statusElement.textContent = 'Unknown';
        statusElement.className = 'text-gray-600';
        console.warn(`Initial check failed for ${computer.name}:`, error.message);
      }
      
      // Wait between checks to prevent binding conflicts
      if (i < computers.length - 1) {
        await new Promise(resolve => setTimeout(resolve, 1000));
      }
    }
  }
  
  isCheckingStatus = false;
}

// Fixed Wake-on-LAN with proper button state management
async function wakeComputer(computerName) {
  const computer = computers.find(c => c.name === computerName || c._id === computerName);
  if (!computer) return;

  const wolButton = document.getElementById(`wol-${computer._id}`);
  const originalText = wolButton.textContent;
  const originalClasses = wolButton.className;
  
  wolButton.disabled = true;
  wolButton.textContent = 'Sending...';
  wolButton.className = 'bg-gray-500 text-white p-2 rounded cursor-pointer text-base';

  try {
    const response = await fetch(`${baseUrl}/wol?mac=${computer.mac}`, {
      method: 'GET',
      headers: { 'Cache-Control': 'no-cache' },
      signal: AbortSignal.timeout(3000)
    });

    if (!response.ok) throw new Error(`HTTP ${response.status}`);

    const data = await response.json();
    if (data.success) {
      wolButton.textContent = 'Sent!';
      wolButton.className = 'bg-blue-500 text-white p-2 rounded cursor-pointer text-base';
      
      // Check status after delay
      setTimeout(() => checkSingleComputerStatus(computer._id), 30000);
      
      setTimeout(() => {
        wolButton.textContent = originalText;
        wolButton.className = originalClasses;
        wolButton.disabled = false;
      }, 1500);
    } else {
      throw new Error('WOL failed');
    }
  } catch (error) {
    wolButton.textContent = 'Failed!';
    wolButton.className = 'bg-red-500 text-white p-2 rounded cursor-pointer text-base';
    
    setTimeout(() => {
      wolButton.textContent = originalText;
      wolButton.className = originalClasses;
      wolButton.disabled = false;
    }, 1500);
    
    console.warn(`WOL failed for ${computer.name}:`, error.message);
  }
}

// Add event listeners to all buttons
function setupComputerButtons() {
  for (const computer of computers) {
    const wolButton = document.getElementById(`wol-${computer._id}`);
    const refreshButton = document.getElementById(`refresh-${computer._id}`);
    
    if (wolButton) {
      wolButton.addEventListener('click', () => wakeComputer(computer._id));
    }
    
    if (refreshButton) {
      refreshButton.addEventListener('click', () => checkSingleComputerStatus(computer._id));
    }
  }
}

// Initialize
createComputerDivs();
setupComputerButtons();

// Initial computer status check - wait a bit for the page to load completely
setTimeout(() => {
  checkComputersOnLoad();
}, 1000);
