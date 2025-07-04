// Check the local storage for theme preference on page load
if (localStorage.getItem('theme') === 'dark' ||
  (!localStorage.getItem('theme') && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
  document.body.classList.add('dark');
} else {
  document.body.classList.remove('dark');
}

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

// Function to update the progress of the temperature and humidity circles
function updateProgress(value, id) {
  // Get the circle and text elements by their IDs
  const circle = document.getElementById(id);
  const text = document.getElementById(id + '-text');

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
      { max: 18, color: 'stroke-blue-500', label: '(Cold)' },
      { max: 24, color: 'stroke-green-500', label: '(Comfortable)' },
      { max: 28, color: 'stroke-yellow-500', label: '(Warm)' },
      { max: 32, color: 'stroke-orange-500', label: '(Hot)' },
      { max: Infinity, color: 'stroke-red-500', label: '(Very Hot)' }
    ],
    h: [
      { max: 30, color: 'stroke-blue-500', label: '(Dry)' },
      { max: 60, color: 'stroke-green-500', label: '(Comfortable)' },
      { max: 70, color: 'stroke-yellow-500', label: '(Humid)' },
      { max: 80, color: 'stroke-orange-500', label: '(Very Humid)' },
      { max: Infinity, color: 'stroke-red-500', label: '(Excessive)' }
    ]
  };

  // Find the appropriate condition based on the value
  const condition = conditions[id].find(cond => (value <= cond.max));

  // Remove previous color classes that start with 'stroke-'
  circle.classList.forEach(cls => {
    if (cls.startsWith('stroke-')) {
      circle.classList.remove(cls);
    }
  });

  // Add the new color class
  circle.classList.add(condition.color);

  // Update the text content with the value and condition label
  text.innerHTML = value + (id === 't' ? '&#8451;' : '&#37') + '<br>' + condition.label;
}

// Get the status text by their element ID
statusText = document.getElementById('status');

// Fetch data from the API, update the progress and change the status every 3 seconds (In development you don't need to fetch data from the API, you can use the updateProgress function to test the progress update)
function getTempAndHumidity() {
  fetch('http://esp32.local/api', {
      method: 'GET',
      headers: {
        'Cache-Control': 'no-cache'
      },
      signal: AbortSignal.timeout(10000) // 5 second timeout for fetch
    })
    .then(response => response.json())
    .then(data => {
      updateProgress(data.temperature, 't');
      updateProgress(data.humidity, 'h');
      statusText.innerHTML = 'Connected';
      statusText.style.color = 'green';
    })
    .catch(error => {
      console.error('Error fetching data:', error);
      statusText.innerHTML = 'Disconnected';
      statusText.style.color = 'red';
    });
}

// Comment this lines if you want to test the progress update function in while development
setInterval(() => {
  getTempAndHumidity();
}, 20000);
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
// Define the computers
const computers = [
  { name: 'home', mac: '08-97-98-EE-02-85', ip: '192.168.1.200'},
  { name: 'home2', mac: '44-39-C4-95-AB-C1', ip: '192.168.1.222'}
];

// Function to create computer divs dynamically
function createComputerDivs() {
  const container = document.getElementById('computer-container');
  // Create computer entries
  computers.forEach((computer, index) => {
    const computerDiv = document.createElement('div');
    const isLast = index === computers.length - 1;
    computerDiv.className = isLast ? '' : 'border-b border-gray-300 dark:border-gray-600';
    
    computerDiv.innerHTML = `
      <div class="grid md:grid-cols-6 grid-cols-1 p-3 gap-3 items-center">
        <div class="px-4 font-medium md:font-normal md:col-span-2">
          <span class="md:hidden font-bold mr-2">Name:</span>${computer.name}
        </div>
        <div class="px-4 md:col-span-1">
          <span class="md:hidden font-bold mr-2">Status:</span><span id="status-${computer.name}" class="text-gray-600 dark:text-gray-400">Unknown</span>
        </div>
        <div class="px-4 flex justify-center gap-2 md:col-span-3">
          <button id="wol-${computer.name}" class="bg-green-500 hover:bg-green-600 text-white py-1 px-3 rounded cursor-pointer">
            Power on
          </button>
        </div>
      </div>
    `;
    
    container.appendChild(computerDiv);
  });
}

// Function to check if a computer is online (using ping via API)
async function checkComputerStatus() {
  const statusPromises = computers.map(async (computer) => {
    try {
      const response = await fetch(`http://esp32.local/ping?ip=${computer.ip}`, {
        method: 'GET',
        headers: {
          'Cache-Control': 'no-cache'
        },
        signal: AbortSignal.timeout(5000) // 5 second timeout for ping
      });
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }
      
      const data = await response.json();

      const statusElement = document.getElementById(`status-${computer.name}`);

      if (data.online) {
        statusElement.textContent = 'Online';
        statusElement.className = 'text-green-600';
      } else {
        statusElement.textContent = 'Offline';
        statusElement.className = 'text-red-600';
      }
    } catch (error) {
      console.error(`Error checking status for ${computer.name}:`, error);
      const statusElement = document.getElementById(`status-${computer.name}`);
      statusElement.textContent = 'Unknown';
      statusElement.className = 'text-gray-600 dark:text-gray-400';
    }
  });
  
  // Wait for all ping requests to complete
  await Promise.allSettled(statusPromises);
}

// Function to send Wake on LAN magic packet
async function wakeComputer(computerName) {
  const computer = computers.find(c => c.name === computerName);
  if (!computer) return;

  try {
    const response = await fetch(`http://esp32.local/wol?mac=${computer.mac}`, {
      method: 'GET',
      headers: {
        'Cache-Control': 'no-cache'
      },
      signal: AbortSignal.timeout(5000) // 5 second timeout for WOL
    });

    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }

    const data = await response.json();
    if (data.success) {
      alert(`Wake-on-LAN packet sent to ${computer.name}`);
    } else {
      alert(`Failed to send Wake-on-LAN packet to ${computer.name}: ${data.message || 'Unknown error'}`);
    }
  } catch (error) {
    console.error(`Error sending WOL to ${computer.name}:`, error);
    alert(`Error sending Wake-on-LAN packet to ${computer.name}: ${error.message}`);
  }
}

// Add event listeners to all WOL and shutdown buttons
function setupComputerButtons() {
  for (const computer of computers) {
    const wolButton = document.getElementById(`wol-${computer.name}`);

    wolButton.addEventListener('click', () => wakeComputer(computer.name));
  }
}

// Initialize
createComputerDivs();
setupComputerButtons();
// checkComputerStatus();

// Check computer status every 10 seconds
// setInterval(() => {
//   checkComputerStatus();
// }, 7000);
