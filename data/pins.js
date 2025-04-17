// This file contains all the pin definitions for the ESP32-S3

/* @tweakable whether to automatically hide used pins from other categories */
const autoHideUsedPins = true;

/* @tweakable whether to remove I2C/SPI pins from all other categories when used */
const removeProtocolPinsFromAllCategories = true;

/* @tweakable pin display colors by capability */
const pinColorScheme = {
    gpio: "#46f27a", // Green
    adc1: "#f2ae46", // Orange
    adc2: "#f2c146", // Dark Orange
    pwm: "#4686f2",  // Blue
    i2c: "#f246e1",  // Pink
    spi: "#f24646",  // Red
    uart: "#9b46f2", // Purple
    usb: "#46f2dd",  // Cyan
    special: "#a1a1a1" // Gray
};

// Define all the pins with their capabilities
const pinDefinitions = [
    // GPIO pins (all pins can be used as GPIO)
    ...Array.from({ length: 22 }, (_, i) => ({ number: i, name: `GPIO${i}`, capabilities: ["gpio", "pwm"] })), // GPIO0 to GPIO21
    ...Array.from({ length: 16 }, (_, i) => ({ number: i + 35, name: `GPIO${i + 35}`, capabilities: ["gpio", "pwm"] })), // GPIO35 to GPIO48

    // ADC specific pins
    { number: 1, name: "ADC1_0", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 2, name: "ADC1_1", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 3, name: "ADC1_2", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 4, name: "ADC1_3", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 5, name: "ADC1_4", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 6, name: "ADC1_5", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 7, name: "ADC1_6", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 8, name: "ADC1_7", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 9, name: "ADC1_8", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 10, name: "ADC1_9", capabilities: ["gpio", "adc", "adc1", "pwm"] },
    { number: 11, name: "ADC2_0", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 12, name: "ADC2_1", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 13, name: "ADC2_2", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 14, name: "ADC2_3", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 15, name: "ADC2_4", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 16, name: "ADC2_5", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 17, name: "ADC2_6", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 18, name: "ADC2_7", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 19, name: "ADC2_8", capabilities: ["gpio", "adc", "adc2", "pwm"] },
    { number: 20, name: "ADC2_9", capabilities: ["gpio", "adc", "adc2", "pwm"] },

    // I2C pins
    { number: 8, name: "SDA", capabilities: ["gpio", "i2c", "pwm"] },
    { number: 9, name: "SCL", capabilities: ["gpio", "i2c", "pwm"] },

    // SPI pins
    { number: 10, name: "HSPI_CS", capabilities: ["gpio", "spi", "pwm"] },
    { number: 11, name: "HSPI_MOSI", capabilities: ["gpio", "spi", "pwm"] },
    { number: 12, name: "HSPI_CLK", capabilities: ["gpio", "spi", "pwm"] },
    { number: 13, name: "HSPI_MISO", capabilities: ["gpio", "spi", "pwm"] },
    { number: 39, name: "VSPI_CS", capabilities: ["gpio", "spi", "pwm"] },
    { number: 35, name: "VSPI_MOSI", capabilities: ["gpio", "spi", "pwm"] },
    { number: 36, name: "VSPI_CLK", capabilities: ["gpio", "spi", "pwm"] },
    { number: 37, name: "VSPI_MISO", capabilities: ["gpio", "spi", "pwm"] },

    // UART pins
    { number: 43, name: "TXD0", capabilities: ["gpio", "uart", "pwm"] },
    { number: 44, name: "RXD0", capabilities: ["gpio", "uart", "pwm"] },
    { number: 17, name: "TXD1", capabilities: ["gpio", "uart", "pwm"] },
    { number: 18, name: "RXD1", capabilities: ["gpio", "uart", "pwm"] },
    

    // Special function pins
    { number: 0, name: "BOOT", capabilities: ["gpio", "strap", "pwm"] },
    { number: 46, name: "LOG", capabilities: ["gpio", "strap", "pwm"] },
    { number: 45, name: "VSPI", capabilities: ["gpio", "strap", "pwm"] },
    { number: 3, name: "JTAG", capabilities: ["gpio", "strap", "pwm"] }
];

// This will keep track of which pins are currently in use along with their capabilities
let usedPins = [];

// Function to mark a pin as used
function usePin(pinNumber, capability) {
    const existingPin = usedPins.find(pin => pin.number === pinNumber);
    if (!existingPin) {
        usedPins.push({ number: pinNumber, capabilities: [capability] });
    }
    updatePinLists();
}

// Function to mark a pin as unused for a specific capability
function releasePin(pinNumber, capability) {
    const existingPin = usedPins.find(pin => pin.number === pinNumber);
    if (existingPin) {
        usedPins = usedPins.filter(pin => pin.number !== pinNumber);
    }
    updatePinLists();
}

// Function to check if a pin is in use for a specific capability
function isPinUsed(pinNumber, capability = null) {
    const existingPin = usedPins.find(pin => pin.number === pinNumber);
    if (!existingPin) return false;
    return true;
}

// function to check if a pin is used for a specific capability
function isPinUsedForCapability(pinNumber, capability) {
    const existingPin = usedPins.find(pin => pin.number === pinNumber);
    if (existingPin) {
        return existingPin.capabilities.includes(capability);
    }
    return false;
}

// Function to get all pins with a specific capability
function getPinsWithCapability(capability) {
    if (capability === 'adc') {
        return pinDefinitions.filter(pin => pin.capabilities.includes('adc'));
    } else {
        return pinDefinitions.filter(pin => pin.capabilities.includes(capability));
    }
}

// Update the pin lists in the UI
function updatePinLists() {
    // Clear all pin lists
    document.getElementById('gpio-pins').innerHTML = '';
    document.getElementById('adc1-pins').innerHTML = '';
    document.getElementById('adc2-pins').innerHTML = '';
    document.getElementById('pwm-pins').innerHTML = '';
    document.getElementById('i2c-pins').innerHTML = '';
    document.getElementById('spi-pins').innerHTML = '';
    document.getElementById('uart-pins').innerHTML = '';
    document.getElementById('special-pins').innerHTML = '';
    
    // Helper function to add pins to a list
    const addPinsToList = (pins, listId, capability) => {
        const listElement = document.getElementById(listId);
        
        // Create a unique set of pin numbers for this capability
        const uniquePins = [];
        pins.forEach(pin => {
            if (!uniquePins.some(p => p.number === pin.number)) {
                uniquePins.push(pin);
            }
        });
        
        // Sort pins by number
        uniquePins.sort((a, b) => a.number - b.number);
        
        // Add each pin to the list
        uniquePins.forEach(pin => {
            if (!autoHideUsedPins || !isPinUsed(pin.number, capability) || ((capability === 'i2c' || capability === 'spi') && isPinUsedForCapability(pin.number, capability))) {
                const pinElement = document.createElement('li');
                pinElement.textContent = `${pin.name} (GPIO${pin.number})`;
                pinElement.dataset.pinNumber = pin.number;
                pinElement.dataset.pinName = pin.name;
                pinElement.dataset.capability = capability;
                
                if (isPinUsed(pin.number) && !isPinUsedForCapability(pin.number, capability)) {
                    pinElement.classList.add('used');
                } else {
                    pinElement.addEventListener('click', () => {
                        selectPin(pin.number, pin.name, capability);
                    });
                }
                
                // Apply color styling based on capability
                if (pinColorScheme[capability]) {
                    pinElement.style.setProperty(`--${capability}-color`, pinColorScheme[capability]);
                }
                
                listElement.appendChild(pinElement);
            }
        });
    };
    
    // Add pins to each category
    addPinsToList(getPinsWithCapability('gpio'), 'gpio-pins', 'gpio');
    addPinsToList(getPinsWithCapability('adc1'), 'adc1-pins', 'adc1'); // Ensure to pass 'adc1' here
    addPinsToList(getPinsWithCapability('adc2'), 'adc2-pins', 'adc2'); // Ensure to pass 'adc2' here
    addPinsToList(getPinsWithCapability('pwm'), 'pwm-pins', 'pwm');
    addPinsToList(getPinsWithCapability('i2c'), 'i2c-pins', 'i2c');
    addPinsToList(getPinsWithCapability('spi'), 'spi-pins', 'spi');
    addPinsToList(getPinsWithCapability('uart'), 'uart-pins', 'uart');
    
    // Add special pins
    const specialPins = [
        ...getPinsWithCapability('strap')
    ];
    addPinsToList(specialPins, 'special-pins', 'special');
}

// Check if a pin is used for I2C or SPI
function isProtocolPin(pinNumber) {
    for (const component of components) {
        for (const pin of component.pins) {
            if (pin.number === pinNumber && (pin.capability === 'i2c' || pin.capability === 'spi')) {
                return true;
            }
        }
    }
    return false;
}

// Initialize pin lists on page load
document.addEventListener('DOMContentLoaded', updatePinLists);