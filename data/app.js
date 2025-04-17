// Main application logic
let components = []; // Stores all added components
let selectedComponent = null; // Currently selected component

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    initComponentList();
    initPinSelection();
    setupEventListeners();
});

// Set up event listeners
function setupEventListeners() {
    // Generate config button
    document.getElementById('generate-config-btn').addEventListener('click', generateConfig);
    
    // Save config button
    document.getElementById('save-config-btn').addEventListener('click', saveConfig);
    
    // Read config button
    document.getElementById('read-config-btn').addEventListener('click', readConfigFromDevice);
    
    // Setup configuration form container for dynamic content
    document.getElementById('config-form-container').addEventListener('click', function(e) {
        if (e.target.classList.contains('remove-pin')) {
            const pinNumber = parseInt(e.target.dataset.pinNumber);
            if (selectedComponent) {
                selectedComponent.removePin(pinNumber);
                updateComponentDetails();
            }
        }
    });
    
    // Listen for changes in component config forms
    document.getElementById('config-form-container').addEventListener('change', function(e) {
        if (selectedComponent && e.target.id === 'sensor-type') {
            const sensorType = e.target.value;
            document.getElementById('sensor-analog-pins').style.display = sensorType === 'Analog' ? 'block' : 'none';
            document.getElementById('sensor-digital-pins').style.display = sensorType === 'Digital' ? 'block' : 'none';
        } else if (selectedComponent && e.target.id === 'imu-protocol') {
            const protocol = e.target.value;
            document.getElementById('imu-address-field').style.display = protocol === 'I2C' ? 'block' : 'none';
            document.getElementById('imu-i2c-pins').style.display = protocol === 'I2C' ? 'block' : 'none';
            document.getElementById('imu-spi-pins').style.display = protocol === 'SPI' ? 'block' : 'none';
        }
        
        if (selectedComponent) {
            selectedComponent.updateFromForm();
        }
    });
}

// Initialize component list with click handlers
function initComponentList() {
    const componentList = document.getElementById('component-list');
    componentList.querySelectorAll('li').forEach(item => {
        item.addEventListener('click', function() {
            const componentType = this.dataset.componentType;
            addComponent(componentType);
        });
    });
}

// Initialize pin selection
function initPinSelection() {
    updatePinLists();
}

// Add a new component
function addComponent(type) {
    const component = componentFactory.createComponent(type);
    components.push(component);
    updateUsedComponentsList();
    selectComponent(component);
}

// Select a component for editing
function selectComponent(component) {
    selectedComponent = component;
    
    // Update UI to show the selected component
    const usedComponentsList = document.getElementById('used-components-list');
    usedComponentsList.querySelectorAll('.used-component').forEach(el => {
        el.classList.remove('selected');
        if (el.dataset.componentIndex === components.indexOf(component).toString()) {
            el.classList.add('selected');
        }
    });
    
    // Show the component's configuration form
    updateComponentDetails();
}

// Select a pin for the current component
function selectPin(pinNumber, pinName, capability) {
    if (selectedComponent) {
        selectedComponent.addPin(pinNumber, pinName, capability);
        updateComponentDetails();
    } else {
        alert('Please select a component first before selecting a pin.');
    }
}

// Update the list of used components
function updateUsedComponentsList() {
    const usedComponentsList = document.getElementById('used-components-list');
    usedComponentsList.innerHTML = '';
    
    components.forEach((component, index) => {
        const componentEl = document.createElement('div');
        componentEl.className = 'used-component';
        if (selectedComponent === component) {
            componentEl.classList.add('selected');
        }
        componentEl.dataset.componentIndex = index;
        
        // Create pin indicator dots for each pin type
        const pinTypeCounts = {};
        component.pins.forEach(pin => {
            pinTypeCounts[pin.capability] = (pinTypeCounts[pin.capability] || 0) + 1;
        });
        
        const pinIndicators = Object.entries(pinTypeCounts).map(([type, count]) => {
            return `<div class="pin-indicator" data-pin-type="${type}" title="${count} ${type} pin${count > 1 ? 's' : ''}"></div>`;
        }).join('');
        
        componentEl.innerHTML = `
            <h3>${component.name} (${component.type})</h3>
            <div>Pins: ${component.pins.map(p => p.name).join(', ') || 'None'}</div>
            <div class="component-pin-indicators">${pinIndicators}</div>
            <div class="component-actions">
                <button class="edit-component">Edit</button>
                <button class="remove-component">Remove</button>
            </div>
        `;
        
        usedComponentsList.appendChild(componentEl);
        
        // Add event listeners
        componentEl.querySelector('.edit-component').addEventListener('click', () => {
            selectComponent(component);
        });
        
        componentEl.querySelector('.remove-component').addEventListener('click', () => {
            removeComponent(index);
        });
    });
}

// Remove a component
function removeComponent(index) {
    const component = components[index];
    
    // Release all pins used by this component
    component.pins.forEach(pin => {
        releasePin(pin.number);
    });
    
    // Remove the component from the array
    components.splice(index, 1);
    
    // Update the UI
    updateUsedComponentsList();
    
    // If the selected component was removed, clear the selection
    if (selectedComponent === component) {
        selectedComponent = null;
        document.getElementById('config-form-container').innerHTML = '<p>Select a component to configure</p>';
    }
}

// Update the component configuration details
function updateComponentDetails() {
    const configContainer = document.getElementById('config-form-container');
    
    if (selectedComponent) {
        configContainer.innerHTML = selectedComponent.getConfigHtml();
    } else {
        configContainer.innerHTML = '<p>Select a component to configure</p>';
    }
}

// Generate the configuration JSON
function generateConfig() {
    const config = {
        wifi: {
            ssid: document.getElementById('wifi-ssid').value,
            password: document.getElementById('wifi-password').value
        },
        components: components.map(component => component.toJSON())
    };
    
    const jsonOutput = document.getElementById('json-output');
    jsonOutput.textContent = JSON.stringify(config, null, 2);
}

// Save the configuration to the ESP32
function saveConfig() {
    const config = {
        wifi: {
            ssid: document.getElementById('wifi-ssid').value,
            password: document.getElementById('wifi-password').value
        },
        components: components.map(component => component.toJSON())
    };

    fetch('/api/save-config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    })
    .then(response => {
        if (response.ok) return response.json();
        throw new Error('Failed to save configuration.');
    })
    .then(() => alert('Configuration saved successfully!'))
    .catch(error => {
        console.error('Error saving configuration:', error);
        alert('Error saving configuration. Please try again.');
    });
}

// Read configuration from the ESP32
function readConfigFromDevice() {
    fetch('/api/read-config')
        .then(response => {
            if (response.ok) return response.json();
            throw new Error('Failed to load configuration.');
        })
        .then(data => {
            // Clear existing components
            components = [];
            usedPins.forEach(pin => releasePin(pin));

            // Populate WiFi settings
            if (data.wifi) {
                document.getElementById('wifi-ssid').value = data.wifi.ssid || '';
                document.getElementById('wifi-password').value = data.wifi.password || '';
            }

            // Populate components
            if (data.components) {
                data.components.forEach(componentData => {
                    const component = componentFactory.createComponent(componentData.type, componentData.name);
                    if (componentData.pins) {
                        componentData.pins.forEach(pin => component.addPin(pin.number, pin.name, pin.capability));
                    }
                    if (componentData.config) {
                        component.config = { ...componentData.config };
                    }
                    components.push(component);
                });
            }

            updateUsedComponentsList();
            alert('Configuration loaded successfully!');
        })
        .catch(error => {
            console.error('Error loading configuration:', error);
            alert('Error loading configuration. Please try again.');
        });
    
    generateConfig();
}