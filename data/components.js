class Component {
    constructor(type, name) {
        this.type = type;
        this.name = name || `${type}_${Math.floor(Math.random() * 1000)}`;
        this.pins = [];
        this.config = {};
        this.config.topicUpdateInterval = 1000; // Default update interval in ms
    }
    
    addPin(pinNumber, pinName, capability) {
        // Validate pin capability
        if (!this.isCapabilityValid(capability)) {
            alert(`Error: Cannot add pin "${pinName}" (GPIO${pinNumber}) with capability "${capability}" to component type "${this.type}".`);
            return;
        }
        this.pins.push({
            number: pinNumber,
            name: pinName,
            capability: capability
        });
        usePin(pinNumber, capability); // Pass capability to handle protocol-specific logic
        
        // Add visual highlighting for the pin type
        const pinElement = document.createElement('span');
        pinElement.className = `pin-indicator pin-${capability}`;
        pinElement.dataset.pinType = capability;
        if (pinColorScheme[capability]) {
            pinElement.style.backgroundColor = pinColorScheme[capability];
        }
        this.pinIndicators = this.pinIndicators || [];
        this.pinIndicators.push(pinElement);
    }

    isCapabilityValid(capability) {
        // Override in subclasses to define valid capabilities
        return true;
    }
    
    removePin(pinNumber) {
        this.pins = this.pins.filter(pin => pin.number !== pinNumber);
        releasePin(pinNumber);
    }
    
    // Get configuration HTML for this component type
    getConfigHtml() {
        return `
            <div class="config-field">
                <label for="component-name">Component Name:</label>
                <input type="text" id="component-name" value="${this.name}">
            </div>
            <div class="config-field">
                <label for="topic-update-interval">Topic Update Interval (ms):</label>
                <input type="text" id="topic-update-interval" value="${this.config.topicUpdateInterval}">
            </div>
        `;
    }
    
    // Update component from config form
    updateFromForm() {
        this.name = document.getElementById('component-name').value;
        this.config.topicUpdateInterval = parseInt(document.getElementById('topic-update-interval').value, 10) || 1000;
    }
    
    // Get JSON representation
    toJSON() {
        return {
            type: this.type,
            name: this.name,
            pins: this.pins.map(pin => ({
                number: pin.number,
                name: pin.name,
                capability: pin.capability
            })),
            config: this.config
        };
    }
}

class ServoComponent extends Component {
    constructor(name) {
        super('Servo', name);
        this.config.type = 'SG90_180'; // Default type
    }
    
    getConfigHtml() {
        return `
            ${super.getConfigHtml()}
            <div class="config-field">
                <label for="servo-type">Servo Type:</label>
                <select id="servo-type">
                    <option value="SG90_180" ${this.config.type === 'SG90_180' ? 'selected' : ''}>SG90_180 (180°)</option>
                    <option value="SG90_360" ${this.config.type === 'SG90_360' ? 'selected' : ''}>SG90_360 (360°)</option>
                </select>
            </div>
            <div class="config-field">
                <label>PWM Pin:</label>
                ${this.renderPinSelector('pwm')}
            </div>
        `;
    }
    
    renderPinSelector(capability) {
        const pin = this.pins.find(p => p.capability === capability);
        
        if (pin) {
            return `
                <div>
                    <span>${pin.name} (GPIO${pin.number})</span>
                    <button class="remove-pin" data-pin-number="${pin.number}">Remove</button>
                </div>
            `;
        } else {
            return '<div>No pin selected. Please select a pin from the PWM Pins list.</div>';
        }
    }
    
    updateFromForm() {
        super.updateFromForm();
        this.config.type = document.getElementById('servo-type').value;
    }

    isCapabilityValid(capability) {
        return capability === 'pwm';
    }
}

class IMUComponent extends Component {
    constructor(name) {
        super('IMU', name);
        this.config.protocol = 'I2C';
        this.config.address = '0x68';
    }
    
    getConfigHtml() {
        return `
            ${super.getConfigHtml()}
            <div class="config-field">
                <label for="imu-protocol">Protocol:</label>
                <select id="imu-protocol">
                    <option value="I2C" ${this.config.protocol === 'I2C' ? 'selected' : ''}>I2C</option>
                    <option value="SPI" ${this.config.protocol === 'SPI' ? 'selected' : ''}>SPI</option>
                </select>
            </div>
            <div class="config-field" id="imu-address-field" style="${this.config.protocol === 'I2C' ? '' : 'display: none;'}">
                <label for="imu-address">I2C Address:</label>
                <input type="text" id="imu-address" value="${this.config.address}">
            </div>
            <div class="config-field" id="imu-i2c-pins" style="${this.config.protocol === 'I2C' ? '' : 'display: none;'}">
                <label>I2C Pins:</label>
                ${this.renderPinSelector('i2c')}
            </div>
            <div class="config-field" id="imu-spi-pins" style="${this.config.protocol === 'SPI' ? '' : 'display: none;'}">
                <label>SPI Pins:</label>
                ${this.renderPinSelector('spi')}
            </div>
        `;
    }
    
    renderPinSelector(capability) {
        const pins = this.pins.filter(p => p.capability === capability);
        
        if (pins.length > 0) {
            return pins.map(pin => `
                <div>
                    <span>${pin.name} (GPIO${pin.number})</span>
                    <button class="remove-pin" data-pin-number="${pin.number}">Remove</button>
                </div>
            `).join('');
        } else {
            return `<div>No pins selected. Please select pins from the ${capability.toUpperCase()} Pins list.</div>`;
        }
    }
    
    updateFromForm() {
        super.updateFromForm();
        this.config.protocol = document.getElementById('imu-protocol').value;
        if (this.config.protocol === 'I2C') {
            this.config.address = document.getElementById('imu-address').value;
        }
    }

    isCapabilityValid(capability) {
        // Validate based on the selected protocol
        if (this.config.protocol === 'I2C') {
            return capability === 'i2c';
        } else if (this.config.protocol === 'SPI') {
            return capability === 'spi';
        }
        return false;
    }
}

class LEDComponent extends Component {
    constructor(name) {
        super('LED', name);
    }
    
    getConfigHtml() {
        return `
            ${super.getConfigHtml()}
            <div class="config-field">
                <label>GPIO Pin:</label>
                ${this.renderPinSelector('gpio')}
            </div>
        `;
    }
    
    renderPinSelector(capability) {
        const pin = this.pins.find(p => p.capability === capability);
        
        if (pin) {
            return `
                <div>
                    <span>${pin.name} (GPIO${pin.number})</span>
                    <button class="remove-pin" data-pin-number="${pin.number}">Remove</button>
                </div>
            `;
        } else {
            return '<div>No pin selected. Please select a pin from the GPIO Pins list.</div>';
        }
    }

    isCapabilityValid(capability) {
        return capability === 'gpio';
    }
}

class ButtonComponent extends Component {
    constructor(name) {
        super('Button', name);
        this.config.pullup = true;
    }
    
    getConfigHtml() {
        return `
            ${super.getConfigHtml()}
            <div class="config-field">
                <label for="button-pullup">Pull-up Resistor:</label>
                <select id="button-pullup">
                    <option value="true" ${this.config.pullup ? 'selected' : ''}>Enabled</option>
                    <option value="false" ${!this.config.pullup ? 'selected' : ''}>Disabled</option>
                </select>
            </div>
            <div class="config-field">
                <label>GPIO Pin:</label>
                ${this.renderPinSelector('gpio')}
            </div>
        `;
    }
    
    renderPinSelector(capability) {
        const pin = this.pins.find(p => p.capability === capability);
        
        if (pin) {
            return `
                <div>
                    <span>${pin.name} (GPIO${pin.number})</span>
                    <button class="remove-pin" data-pin-number="${pin.number}">Remove</button>
                </div>
            `;
        } else {
            return '<div>No pin selected. Please select a pin from the GPIO Pins list.</div>';
        }
    }
    
    updateFromForm() {
        super.updateFromForm();
        this.config.pullup = document.getElementById('button-pullup').value === 'true';
    }

    isCapabilityValid(capability) {
        return capability === 'gpio';
    }
}

class SensorComponent extends Component {
    constructor(name) {
        super('Sensor', name);
        this.config.type = 'Analog';
    }
    
    getConfigHtml() {
        return `
            ${super.getConfigHtml()}
            <div class="config-field">
                <label for="sensor-type">Sensor Type:</label>
                <select id="sensor-type">
                    <option value="Analog" ${this.config.type === 'Analog' ? 'selected' : ''}>Analog</option>
                    <option value="Digital" ${this.config.type === 'Digital' ? 'selected' : ''}>Digital</option>
                </select>
            </div>
            <div class="config-field" id="sensor-analog-pins" style="${this.config.type === 'Analog' ? '' : 'display: none;'}">
                <label>ADC Pin:</label>
                ${this.renderPinSelector('adc')}
            </div>
            <div class="config-field" id="sensor-digital-pins" style="${this.config.type === 'Digital' ? '' : 'display: none;'}">
                <label>GPIO Pin:</label>
                ${this.renderPinSelector('gpio')}
            </div>
        `;
    }
    
    renderPinSelector(capability) {
        const pin = this.pins.find(p => p.capability === capability);
        
        if (pin) {
            return `
                <div>
                    <span>${pin.name} (GPIO${pin.number})</span>
                    <button class="remove-pin" data-pin-number="${pin.number}">Remove</button>
                </div>
            `;
        } else {
            return `<div>No pin selected. Please select a pin from the ${capability.toUpperCase()} Pins list.</div>`;
        }
    }
    
    updateFromForm() {
        super.updateFromForm();
        this.config.type = document.getElementById('sensor-type').value;
    }

    isCapabilityValid(capability) {
        return capability === 'adc' || capability === 'gpio' || capability === 'adc1' || capability === 'adc2';
    }
}

class HC_SR04Component extends Component {
    constructor(name) {
        super('HC_SR04', name);
    }

        getConfigHtml() {
        return `
            ${super.getConfigHtml()}
            <div class="config-field">
                <label> Trigger Pin:</label>
                ${this.renderPinSelector('trigger')}
            </div>
            <div class="config-field">
                <label> Echo Pin:</label>
                ${this.renderPinSelector('echo')}
            </div>
        `;
    }
    
    renderPinSelector(capability) {
        const pin = this.pins.find(p => p.capability === capability);
        
        if (pin) {
            return `
                <div>
                    <span>${pin.name} (GPIO${pin.number})</span>
                    <button class="remove-pin" data-pin-number="${pin.number}">Remove</button>
                </div>
            `;
        } else {
            return '<div>No pin selected. Please select a pin from the PWM Pins list.</div>';
        }
    }

    isCapabilityValid(capability) {
        return capability === 'gpio';
    }
}
    

/* @tweakable the component factory settings */
const componentFactory = {
    createComponent: function(type, name) {
        switch(type) {
            case 'Servo':
                return new ServoComponent(name);
            case 'IMU':
                return new IMUComponent(name);
            case 'LED':
                return new LEDComponent(name);
            case 'Button':
                return new ButtonComponent(name);
            case 'Sensor':
                return new SensorComponent(name);
            case 'HC_SR04':
                return new HC_SR04Component(name);
            default:
                return new Component(type, name);
        }
    }
};