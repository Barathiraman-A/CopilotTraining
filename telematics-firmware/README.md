# Vehicle Telematics Unit Firmware

## Overview

This repository contains the embedded firmware for a vehicle telematics unit designed to capture, process, and securely transmit critical vehicle telemetry data including:
- Real-time vehicle speed (via CAN bus)
- Battery voltage level (via ADC)
- GPS coordinates and positioning data

## Key Features

- **Low-Power Operation**: Advanced power management with multiple sleep modes
- **Fault Tolerance**: Watchdog monitoring, sensor health checks, and graceful degradation
- **Dual Communication**: Cellular (LTE-M/NB-IoT) and LoRaWAN support with automatic fallback
- **Efficient Data Handling**: Circular buffering with compression and intelligent queuing
- **Secure Transmission**: Encrypted data payloads with integrity checks
- **OTA Updates**: Over-the-air firmware update capability

## Target Platform

- **Microcontroller**: STM32L476RG (ARM Cortex-M4, 80 MHz, ultra-low-power)
- **Memory**: 1 MB Flash, 128 KB SRAM
- **Communication**: Cellular modem (Quectel BG96) + LoRaWAN transceiver (SX1276)
- **RTOS**: FreeRTOS v10.5.1

## Project Structure

```
telematics-firmware/
├── docs/                           # Documentation
│   ├── architecture.md             # Firmware architecture and design
│   ├── detailed_design.md          # Detailed component specifications
│   └── integration_guide.md        # Integration and deployment guide
├── src/                            # Source code
│   ├── main.c                      # Main application entry point
│   ├── app/                        # Application layer
│   ├── drivers/                    # Hardware drivers
│   ├── middleware/                 # Middleware components
│   └── hal/                        # Hardware abstraction layer
├── include/                        # Public header files
├── tests/                          # Unit and integration tests
├── build/                          # Build output (generated)
├── config/                         # Configuration files
└── CMakeLists.txt                  # Build system configuration
```

## Building the Firmware

```bash
# Configure the build
cmake -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=config/arm-gcc-toolchain.cmake

# Build the firmware
cmake --build build

# Flash to target device
cmake --build build --target flash
```

## Power Consumption Profile

| Mode           | Current Draw | Description                          |
|----------------|--------------|--------------------------------------|
| Active Sampling| 45 mA        | All sensors active, data collection  |
| Idle Mode      | 8 mA         | CPU sleep, peripherals on            |
| Deep Sleep     | 2.5 µA       | RTC + wake sources only              |
| Transmission   | 180 mA (peak)| Cellular transmission active         |

## Communication Protocols

### Cellular (Primary)
- LTE-M/NB-IoT via Quectel BG96 modem
- MQTT over TLS 1.2
- Transmission interval: 30s (configurable)

### LoRaWAN (Backup/Low-Power)
- Class A device operation
- SF7-SF12 adaptive data rate
- Transmission interval: 5 minutes (configurable)

## Safety and Compliance

- ISO 26262 ASIL-B design considerations
- Automotive temperature range: -40°C to +85°C
- EMC compliance: ISO 11452-2, ISO 7637-2
- Watchdog protection with 2-second timeout

## License

Proprietary - All rights reserved

## Contact

For technical support or questions, contact the embedded systems team.
