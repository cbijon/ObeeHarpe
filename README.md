# ESP8266 Harpe Device with OTA and ESP-NOW

This Arduino sketch is designed for an ESP8266-based device using the Lolin D1 Mini and an INA219 sensor to monitor power activity. The device can send data to a central receiver using the ESP-NOW protocol and supports Over-The-Air (OTA) updates for firmware changes.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Contributing](#contributing)
- [License](#license)

## Prerequisites

Before you can use this code, ensure you have the following:

- Arduino IDE with ESP8266 board support.
- Lolin D1 Mini or a compatible ESP8266-based board.
- INA219 sensor connected to the board.
- A Wi-Fi network to which the device can connect.
- Central receiver for receiving ESP-NOW data.

## Installation

1. Clone or download this repository to your local machine.

2. Open the Arduino IDE and install the required libraries if not already installed. These include:
   - `Adafruit_INA219` for interfacing with the INA219 sensor.
   - `esp8266` for ESP8266 support.
   - `RemoteDebug` for remote debugging (optional).

3. Connect your ESP8266 board to your computer using a USB cable.

4. Open the Arduino sketch (`ESP8266_Harpe_Device.ino`) from the cloned repository.

5. Customize the configuration parameters as needed. (See [Configuration](#configuration) section below)

6. Upload the sketch to your ESP8266 board.

7. If you plan to use OTA updates, make sure to configure your Wi-Fi credentials and provide a password for OTA updates.

8. Once the code is uploaded, the device will operate as intended, monitoring power activity and sending data over ESP-NOW.

## Usage

- The device will start monitoring power activity once powered on.
- Data is periodically sent to the central receiver over ESP-NOW.
- If OTA updates are enabled, you can remotely update the device's firmware using the Arduino IDE and the correct OTA password.
- Remote debugging is available for easier issue diagnosis.

## Configuration

The code can be customized by modifying the following parameters in the Arduino sketch:

- `BoardID`: A unique identifier for each device.
- `HOSTNAME`: Hostname for better identification.
- `ssid` and `password`: Wi-Fi network credentials.
- `ATO_PASSWORD`: Password for OTA updates.
- `POWER_THRESHOLD`: Threshold value to detect abnormal power activity.
- MAC addresses for ESP-NOW communication, depending on the device mode (AP or STA).

## Contributing

Contributions to this project are welcome. You can contribute by:

- Forking the repository.
- Making your changes or improvements.
- Submitting a pull request.

Please ensure that your code follows the project's coding standards and conventions.

## License

This project is licensed under the [MIT License](LICENSE), which means you are free to use, modify, and distribute the code for personal and commercial projects. However, it comes with no warranties or guarantees of support.
