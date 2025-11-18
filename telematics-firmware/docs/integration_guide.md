# Vehicle Telematics Unit - Integration Guide

**Document Version:** 1.0  
**Date:** November 18, 2025  
**Author:** Embedded Systems Team

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Environment Setup](#development-environment-setup)
3. [Hardware Setup](#hardware-setup)
4. [Building the Firmware](#building-the-firmware)
5. [Flashing and Debugging](#flashing-and-debugging)
6. [Configuration](#configuration)
7. [Testing and Validation](#testing-and-validation)
8. [Deployment](#deployment)
9. [Troubleshooting](#troubleshooting)

---

## 1. Getting Started

### 1.1 Prerequisites

**Required Knowledge**:
- Embedded C programming
- ARM Cortex-M architecture
- Real-time operating systems (FreeRTOS)
- CAN bus protocol basics
- Basic understanding of GPS/GNSS

**Required Hardware**:
- STM32L476RG development board (NUCLEO-L476RG or custom PCB)
- ST-LINK/V2 programmer/debugger
- CAN transceiver (MCP2551 or TJA1050)
- GPS module (u-blox NEO-M8N or compatible)
- Cellular modem (Quectel BG96)
- LoRaWAN transceiver (SX1276)
- External SPI flash (W25Q32 or compatible)
- 12V battery simulator or power supply
- CAN analyzer (optional but recommended)

**Required Software**:
- ARM GCC Toolchain (version 10.3 or later)
- CMake (version 3.20 or later)
- OpenOCD (version 0.11 or later)
- Serial terminal (PuTTY, TeraTerm, or minicom)
- Git version control

---

## 2. Development Environment Setup

### 2.1 Windows Setup

**Step 1: Install ARM GCC Toolchain**
```powershell
# Download from: https://developer.arm.com/downloads/-/gnu-rm
# Install to: C:\Program Files (x86)\GNU Arm Embedded Toolchain

# Add to PATH
$env:PATH += ";C:\Program Files (x86)\GNU Arm Embedded Toolchain\10.3 2021.10\bin"
```

**Step 2: Install CMake**
```powershell
# Download from: https://cmake.org/download/
# Or use Chocolatey
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
```

**Step 3: Install OpenOCD**
```powershell
# Download pre-built binaries from: https://gnutoolchains.com/arm-eabi/openocd/
# Extract to: C:\OpenOCD
$env:PATH += ";C:\OpenOCD\bin"
```

**Step 4: Install Build Tools**
```powershell
# Install Ninja build system
choco install ninja

# Or use Visual Studio Build Tools
# Download from: https://visualstudio.microsoft.com/downloads/
```

**Step 5: Verify Installation**
```powershell
arm-none-eabi-gcc --version
cmake --version
openocd --version
ninja --version
```

### 2.2 Linux Setup

**Step 1: Install Dependencies**
```bash
sudo apt-get update
sudo apt-get install -y \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    cmake \
    ninja-build \
    openocd \
    git \
    python3
```

**Step 2: Verify Installation**
```bash
arm-none-eabi-gcc --version
cmake --version
openocd --version
```

### 2.3 IDE Configuration (Optional)

**Visual Studio Code**:
1. Install extensions:
   - C/C++ (Microsoft)
   - CMake Tools (Microsoft)
   - Cortex-Debug (marus25)

2. Configure `settings.json`:
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/config/arm-gcc-toolchain.cmake"
    ],
    "cortex-debug.armToolchainPath": "C:/Program Files (x86)/GNU Arm Embedded Toolchain/10.3 2021.10/bin"
}
```

---

## 3. Hardware Setup

### 3.1 Development Board Connections

**STM32L476RG Pin Assignments**:

| Function | Pin | Connection |
|----------|-----|------------|
| CAN_RX | PB8 | CAN transceiver RX |
| CAN_TX | PB9 | CAN transceiver TX |
| GPS_RX | PA3 | GPS module TX |
| GPS_TX | PA2 | GPS module RX |
| ADC_BAT | PA0 | Battery voltage divider |
| SPI1_SCK | PA5 | SPI flash + LoRa |
| SPI1_MISO | PA6 | SPI flash + LoRa |
| SPI1_MOSI | PA7 | SPI flash + LoRa |
| SPI1_CS_FLASH | PA4 | SPI flash chip select |
| SPI1_CS_LORA | PB6 | LoRa chip select |
| UART3_TX | PC10 | Cellular modem RX |
| UART3_RX | PC11 | Cellular modem TX |
| DEBUG_TX | PA9 | UART debug output |
| DEBUG_RX | PA10 | UART debug input |

**Power Supply**:
- Connect 12V to voltage regulator input
- 5V output to development board VIN
- 3.3V from board to peripherals

**Voltage Divider for Battery Monitoring**:
```
12V ──[100kΩ]──┬──[10kΩ]── GND
               │
               └─── PA0 (ADC input, max 3.3V)

Scaling factor: 11:1 (measures up to 36V)
```

### 3.2 CAN Bus Setup

**Wiring**:
```
STM32           CAN             CAN Bus
PB9 (TX) ───→ TXD              ┌──────┐
PB8 (RX) ←─── RXD    CANH ────→│      │
GND ─────────  GND   CANL ────→│Vehicle│
3.3V ────────  VCC              └──────┘
                │
              120Ω (termination resistor)
```

**CAN Transceiver Configuration**:
- Bitrate: 500 kbps
- Termination: 120Ω resistors at both ends of bus
- Mode: Normal (high-speed CAN)

### 3.3 GPS Module Connection

**UART Configuration**:
- Baud rate: 9600
- Data: 8 bits
- Parity: None
- Stop bits: 1

**Antenna Placement**:
- Position antenna with clear sky view
- Avoid metal obstructions
- Mount on vehicle roof for optimal reception

### 3.4 Cellular Modem Setup

**Quectel BG96 Configuration**:
```
STM32           Modem
PC10 (TX) ───→ RXD
PC11 (RX) ←─── TXD
GND ─────────  GND
3.8V ────────  VBAT
PC12 ────────  PWRKEY (power control)
```

**SIM Card**: Insert activated SIM with data plan (LTE-M or NB-IoT)

---

## 4. Building the Firmware

### 4.1 Clone Repository

```bash
git clone https://github.com/your-org/telematics-firmware.git
cd telematics-firmware
```

### 4.2 Configure Build

**For STM32L476RG target**:
```bash
cmake -B build -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE=config/arm-gcc-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release
```

**Build options**:
- `Release`: Optimized for size and speed
- `Debug`: Debug symbols, no optimization
- `MinSizeRel`: Optimized for minimum size

### 4.3 Build Firmware

```bash
cmake --build build
```

**Build artifacts**:
- `telematics-firmware.elf` - ELF executable with debug info
- `telematics-firmware.hex` - Intel HEX format for flashing
- `telematics-firmware.bin` - Raw binary format
- `telematics-firmware.map` - Memory map file

### 4.4 Check Memory Usage

```bash
cmake --build build --target size
```

**Expected output**:
```
   text    data     bss     dec     hex filename
 450560    2048   46080  498688   79c00 telematics-firmware.elf
```

**Memory budget**:
- Flash: 512 KB allocated (440 KB used = 86%)
- SRAM: 128 KB allocated (46 KB used = 36%)

---

## 5. Flashing and Debugging

### 5.1 Flash via OpenOCD

**Connect ST-LINK to development board**

**Flash command**:
```bash
cmake --build build --target flash
```

**Manual flashing**:
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
    -c "program build/telematics-firmware.elf verify reset exit"
```

### 5.2 Serial Console Debug

**Connect USB-to-UART adapter**:
- RX → PA9 (STM32 TX)
- TX → PA10 (STM32 RX)
- GND → GND

**Open serial terminal**:
```bash
# Linux
minicom -D /dev/ttyUSB0 -b 115200

# Windows
# Use PuTTY: COM port, 115200 baud, 8N1
```

**Expected output**:
```
==========================================
  Vehicle Telematics Unit Firmware
  Version: 1.0.0
  Build Date: 2025-11-18
==========================================

System initialization complete
All tasks created successfully
Starting FreeRTOS scheduler...
SensorAcquisitionTask started
DataProcessingTask started
CommunicationTxTask started
PowerManagementTask started
HealthMonitorTask started
```

### 5.3 GDB Debugging

**Start OpenOCD server**:
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg
```

**Connect GDB (in another terminal)**:
```bash
arm-none-eabi-gdb build/telematics-firmware.elf
(gdb) target extended-remote localhost:3333
(gdb) load
(gdb) monitor reset halt
(gdb) break main
(gdb) continue
```

**Useful GDB commands**:
```gdb
info threads          # List FreeRTOS tasks
thread 2              # Switch to task 2
bt                    # Backtrace
print variable_name   # Print variable value
x/16x 0x20000000      # Examine memory
```

---

## 6. Configuration

### 6.1 System Configuration

Edit `include/config.h`:

**Sensor configuration**:
```c
#define CAN_BITRATE             500000      // CAN bus speed
#define ADC_SAMPLING_RATE_HZ    10          // Battery voltage sampling
#define GPS_UPDATE_RATE_HZ      1           // GPS position rate
#define BATTERY_LOW_THRESHOLD_V 11.5f       // Low battery alert
```

**Communication configuration**:
```c
#define CELLULAR_TX_INTERVAL_MS 30000       // 30 seconds
#define LORAWAN_TX_INTERVAL_MS  300000      // 5 minutes
#define MQTT_BROKER_HOST        "mqtt.example.com"
#define MQTT_BROKER_PORT        8883
```

**Power management**:
```c
#define IDLE_TIMEOUT_MS         30000       // 30 seconds to IDLE
#define SLEEP_TIMEOUT_MS        300000      // 5 minutes to DEEP_SLEEP
```

### 6.2 LoRaWAN Credentials

Create `config/lorawan_keys.h`:
```c
// OTAA Keys (from TTN or other LoRaWAN provider)
static const uint8_t LORAWAN_APP_EUI[8] = {0x00, 0x00, ...};
static const uint8_t LORAWAN_APP_KEY[16] = {0x00, 0x00, ...};
static const uint8_t LORAWAN_DEV_EUI[8] = {0x00, 0x00, ...};
```

### 6.3 Cellular APN Configuration

Edit cellular modem configuration:
```c
#define CELLULAR_APN            "your.apn.com"
#define CELLULAR_APN_USER       "username"
#define CELLULAR_APN_PASS       "password"
```

### 6.4 Rebuild After Configuration

```bash
cmake --build build --target clean
cmake --build build
```

---

## 7. Testing and Validation

### 7.1 Sensor Testing

**CAN Bus Test**:
1. Connect to vehicle CAN bus or CAN simulator
2. Send speed message (ID 0x200) with test data
3. Verify speed extraction in debug console

**Battery Voltage Test**:
1. Apply known voltage (12.0V, 13.8V, 11.0V)
2. Compare ADC reading with multimeter measurement
3. Verify accuracy within ±0.1V

**GPS Test**:
1. Place antenna with clear sky view
2. Wait for GPS fix (LED should blink)
3. Verify coordinates using debug console
4. Compare with known location (< 5m error)

### 7.2 Communication Testing

**Cellular Test**:
```bash
# Monitor cellular modem AT commands
# Connect to modem UART and observe:
AT
AT+CSQ          # Signal quality
AT+COPS?        # Network operator
AT+QMTCONN?     # MQTT connection status
```

**LoRaWAN Test**:
1. Register device on LoRaWAN network (TTN, Helium, etc.)
2. Monitor network console for join request
3. Verify uplink messages appear
4. Check RSSI and SNR values

### 7.3 Power Consumption Test

**Equipment needed**:
- Current measurement shunt (0.1Ω)
- Multimeter or oscilloscope
- Power supply (12V, 500mA minimum)

**Measurement procedure**:
1. Insert shunt resistor in power supply path
2. Measure voltage drop across shunt
3. Calculate current: I = V / 0.1Ω

**Expected measurements**:
- ACTIVE mode: 40-50 mA
- IDLE mode: 7-9 mA
- DEEP_SLEEP mode: < 5 µA

### 7.4 Stress Testing

**Long-duration test** (72 hours minimum):
- Monitor for memory leaks
- Check error log accumulation
- Verify watchdog does not trigger
- Confirm data integrity

**Temperature cycling**:
- Test at -20°C, +25°C, +60°C
- Verify functionality at all temperatures
- Check for timing issues

---

## 8. Deployment

### 8.1 Production Programming

**Flash bootloader first**:
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
    -c "program bootloader.hex verify reset exit"
```

**Flash application firmware**:
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
    -c "program telematics-firmware.hex verify reset exit"
```

**Enable read protection** (prevents firmware extraction):
```bash
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
    -c "init" \
    -c "stm32l4x option_write 0 0x55AA RDPROT 0xBB" \
    -c "exit"
```

### 8.2 Vehicle Installation

**Physical installation**:
1. Mount telematics unit in protected location (under dashboard)
2. Route cables away from high-voltage systems
3. Secure GPS antenna on roof
4. Connect to vehicle 12V battery via fuse (5A recommended)

**CAN bus connection**:
1. Identify CANH and CANL wires (typically orange/blue pair)
2. Use T-tap connectors for non-invasive connection
3. Install termination resistor if required

**Final checks**:
- [ ] All connections secure
- [ ] GPS antenna has sky view
- [ ] Cellular antenna properly installed
- [ ] Power supply stable (11-14V)
- [ ] No short circuits

### 8.3 Commissioning

**Step 1: Power-on test**
- Apply 12V power
- Verify LED patterns
- Check current draw

**Step 2: Sensor verification**
- Start vehicle engine
- Verify CAN speed updates
- Check battery voltage reading
- Wait for GPS fix (may take 2-3 minutes)

**Step 3: Communication test**
- Verify cellular registration (may take 30s)
- Confirm LoRaWAN join (if using)
- Monitor backend for data reception

**Step 4: Road test**
- Drive vehicle for 10 minutes
- Verify continuous data transmission
- Check GPS track on backend map
- Confirm no error alerts

---

## 9. Troubleshooting

### 9.1 Build Issues

**Problem**: `arm-none-eabi-gcc: command not found`

**Solution**:
```bash
# Add toolchain to PATH
export PATH=$PATH:/path/to/arm-gcc/bin

# Or set in CMake
cmake -DCMAKE_C_COMPILER=/path/to/arm-none-eabi-gcc ...
```

**Problem**: `linker script not found`

**Solution**:
Verify `STM32L476RGTx_FLASH.ld` exists in project root

---

### 9.2 Flashing Issues

**Problem**: `Error: init mode failed (unable to connect to target)`

**Solution**:
1. Check ST-LINK connection
2. Verify target has power
3. Try holding RESET button while flashing
4. Check NRST pin is not held low

**Problem**: `Error: flash write failed`

**Solution**:
```bash
# Erase flash first
openocd -f interface/stlink.cfg -f target/stm32l4x.cfg \
    -c "init" -c "reset halt" -c "flash erase_sector 0 0 last" -c "exit"
```

---

### 9.3 Runtime Issues

**Problem**: No GPS fix

**Checklist**:
- [ ] Antenna has clear sky view
- [ ] UART baud rate correct (9600)
- [ ] NMEA sentences being received (check UART debug)
- [ ] Wait longer (cold start can take 5+ minutes)

**Problem**: CAN timeout errors

**Checklist**:
- [ ] CAN transceiver powered
- [ ] CANH/CANL properly connected
- [ ] Termination resistors present (120Ω each end)
- [ ] Bit rate matches vehicle (500 kbps typical)
- [ ] Vehicle is powered on

**Problem**: High current consumption

**Checklist**:
- [ ] GPS module not in standby
- [ ] Cellular modem not entering sleep
- [ ] Power management task running
- [ ] No peripherals stuck active

**Problem**: Watchdog resets

**Checklist**:
- [ ] All tasks running (check debug output)
- [ ] No infinite loops blocking tasks
- [ ] HealthMonitorTask refreshing watchdog
- [ ] FreeRTOS heap not exhausted

---

### 9.4 Communication Issues

**Problem**: Cellular modem not connecting

**Debug steps**:
```bash
# Check modem power and status
AT
AT+CFUN?         # Modem functionality
AT+CPIN?         # SIM card status
AT+CSQ           # Signal quality (>10 is good)
AT+COPS?         # Network registration
AT+CGACT?        # PDP context activation
```

**Problem**: LoRaWAN join fails

**Checklist**:
- [ ] Device registered on network server
- [ ] AppEUI, AppKey, DevEUI correct
- [ ] LoRaWAN gateway in range (< 2 km urban, < 15 km rural)
- [ ] Antenna properly connected
- [ ] Frequency plan matches region

---

## 10. Additional Resources

**Datasheets**:
- STM32L476RG: [Link to datasheet]
- Quectel BG96: [Link to datasheet]
- SX1276: [Link to datasheet]

**Application Notes**:
- AN4660: Using the FreeRTOS with STM32 MPUs
- AN5051: STM32L4 low-power modes

**Support**:
- Technical questions: support@yourcompany.com
- Bug reports: GitHub Issues
- Documentation: https://docs.yourcompany.com

---

**End of Document**
