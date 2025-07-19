# PercipiLabs ESP32 Rain Gauge IoT Device

An ESP32-based IoT rain gauge system that collects environmental sensor data and transmits it to the PercipiLabs API platform for monitoring and analysis.

## Overview

This project implements a smart rain gauge system using an ESP32 microcontroller that measures:

- **Rainfall/Water Level**: Using ultrasonic distance sensor
- **Temperature & Humidity**: Using DHT11 sensor
- **CPU Temperature**: Internal ESP32 temperature sensor
- **Device Runtime**: System uptime tracking

The device securely authenticates with the PercipiLabs API platform and transmits sensor data every 10 seconds for real-time environmental monitoring.

## Hardware Requirements

- **ESP32 Development Board**
- **DHT11 Temperature & Humidity Sensor**
- **HC-SR04 Ultrasonic Distance Sensor** (for rainfall/water level measurement)
- **Breadboard and Jumper Wires**
- **Power Supply** (USB or external)

## Pin Configuration

| Component          | ESP32 Pin |
| ------------------ | --------- |
| DHT11 Data Pin     | GPIO 15   |
| Ultrasonic Trigger | GPIO 4    |
| Ultrasonic Echo    | GPIO 2    |

## Circuit Diagram

```text
ESP32           DHT11
GPIO 15   <-->  Data Pin
3.3V      <-->  VCC
GND       <-->  GND

ESP32           HC-SR04
GPIO 4    <-->  Trig
GPIO 2    <-->  Echo
5V        <-->  VCC
GND       <-->  GND
```

## Software Dependencies

### Arduino Libraries

- `WiFi.h` - ESP32 WiFi connectivity
- `HTTPClient.h` - HTTP requests
- `ArduinoJson.h` - JSON serialization/deserialization
- `DHT.h` - DHT sensor library
- `math.h` - Mathematical functions

### Installation

1. Install the Arduino IDE or PlatformIO
2. Install the ESP32 board package
3. Install required libraries via Library Manager:
   - DHT sensor library by Adafruit
   - ArduinoJson by Benoit Blanchon

## Configuration

### WiFi Setup

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### API Endpoint

```cpp
// For production
const char* BASE_URL = "https://api.percipilabs.me/api";

// For local development
const char* BASE_URL = "http://192.168.1.68:3000/api";
```

### Device Configuration

```cpp
#define DEVICE_NAME "ESP32-RainGauge"
```

## Features

### Secure Authentication

- **Device Registration**: Automatic registration using MAC address
- **JWT Token Management**: Access and refresh token handling
- **Automatic Re-authentication**: Handles token expiration gracefully

### Sensor Data Collection

- **Environmental Monitoring**: Temperature, humidity, and distance measurements
- **Data Validation**: Error handling for sensor failures
- **Real-time Transmission**: 10-second data intervals

### Network Management

- **WiFi Auto-reconnection**: Handles network disconnections
- **HTTP Error Handling**: Robust API communication
- **Connection Status Monitoring**: Real-time network status

### Device Management

- **Runtime Tracking**: System uptime monitoring
- **CPU Temperature**: Internal temperature sensing
- **Serial Debugging**: Comprehensive logging

## API Integration

The device integrates with the PercipiLabs IoT platform through RESTful APIs:

### Authentication Endpoints

- `POST /api/iot/login` - Device authentication with MAC address
- `POST /api/iot/refresh-token` - Token refresh

### Data Transmission

- `POST /api/iot/rainfall-data` - Sensor data submission

### Data Format

```json
{
  "runtime": 12345,
  "temperature": 25.5,
  "humidity": 65.0,
  "cpuTemp": 45.2,
  "distance": 15.7,
  "deviceName": "ESP32-RainGauge"
}
```

## Installation & Setup

1. **Clone the Repository**

   ```bash
   git clone https://github.com/PercipiLabs/percipilabs-iot.git
   cd percipilabs-iot
   ```

2. **Hardware Assembly**

   - Connect sensors according to the pin configuration
   - Ensure proper power supply to all components

3. **Software Configuration**

   - Open `PercipiLabs.ino` in Arduino IDE
   - Update WiFi credentials
   - Configure API endpoint URL
   - Set device name if needed

4. **Upload Code**

   - Select ESP32 board in Arduino IDE
   - Choose correct COM port
   - Upload the sketch

5. **Monitor Operation**
   - Open Serial Monitor (115200 baud)
   - Verify WiFi connection and sensor readings
   - Confirm successful API communication

## Serial Monitor Output

```text
Connecting to WiFi...
WiFi connected!
IP Address: 192.168.1.100
MAC Address: AA:BB:CC:DD:EE:FF
Login request: {"macAddress":"AA:BB:CC:DD:EE:FF"}
Login response: {"access_token":"...", "refresh_token":"..."}
Access Token: eyJ0eXAiOi...
Sending payload: {"runtime":120,"temperature":24.5,"humidity":60,"cpuTemp":42.1,"distance":12.3,"deviceName":"ESP32-RainGauge"}
Data sent successfully
```

## Troubleshooting

### Common Issues

#### WiFi Connection Problems

- Verify SSID and password
- Check signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

#### Sensor Reading Errors

- Check wiring connections
- Verify sensor power supply
- Replace faulty sensors if readings show -999

#### API Communication Issues

- Confirm internet connectivity
- Verify API endpoint URL
- Check firewall settings

#### Authentication Failures

- Ensure device is registered in PercipiLabs platform
- Check MAC address format
- Verify API credentials

## Development

### Code Structure

- `setup()` - Device initialization and authentication
- `loop()` - Main execution cycle (10-second intervals)
- `login()` - Device authentication with API
- `refreshAccessToken()` - Token management
- `sendData()` - Sensor data collection and transmission
- `getDistance()` - Ultrasonic sensor reading
- `getCPUTemperature()` - Internal temperature monitoring

### Customization

- Modify `DEVICE_NAME` for device identification
- Adjust sensor pins in `#define` statements
- Change data transmission interval in `loop()`
- Add additional sensors by extending the JSON payload

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-sensor`)
3. Commit changes (`git commit -am 'Add new sensor support'`)
4. Push to branch (`git push origin feature/new-sensor`)
5. Create Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Support

For technical support and documentation:

- **Website**: [PercipiLabs.me](https://percipilabs.me)
- **Issues**: [GitHub Issues](https://github.com/PercipiLabs/percipilabs-iot/issues)
- **Documentation**: [API Documentation](https://api.percipilabs.me/docs)

## Changelog

### v1.0.0

- Initial release
- ESP32 rain gauge implementation
- DHT11 and ultrasonic sensor support
- PercipiLabs API integration
- JWT authentication system
- WiFi management and error handling

---

**PercipiLabs** - Advancing IoT solutions for environmental monitoring
