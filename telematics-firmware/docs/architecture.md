# Vehicle Telematics Unit - Firmware Architecture

**Document Version:** 1.0  
**Date:** November 18, 2025  
**Author:** Embedded Systems Team

---

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [System Overview](#system-overview)
3. [Hardware Platform](#hardware-platform)
4. [Software Architecture](#software-architecture)
5. [Sensor Interface Layer](#sensor-interface-layer)
6. [Data Acquisition and Buffering](#data-acquisition-and-buffering)
7. [Power Management](#power-management)
8. [Communication Protocol Layer](#communication-protocol-layer)
9. [Fault Tolerance and Diagnostics](#fault-tolerance-and-diagnostics)
10. [Security Architecture](#security-architecture)
11. [Performance Requirements](#performance-requirements)
12. [System State Machine](#system-state-machine)

---

## 1. Executive Summary

This document defines the firmware architecture for a vehicle telematics unit responsible for capturing, processing, and transmitting critical vehicle telemetry data. The design prioritizes:

- **Low Power Consumption**: Target average < 10 mA continuous operation
- **High Reliability**: 99.9% uptime with graceful degradation
- **Real-Time Data Collection**: < 100 ms latency for critical telemetry
- **Secure Communication**: End-to-end encryption for all transmitted data
- **Scalability**: Support for additional sensors and communication protocols

---

## 2. System Overview

### 2.1 Purpose

The telematics unit firmware collects vehicle telemetry (speed, battery level, GPS coordinates) and transmits data to a backend system via cellular or LoRaWAN networks. The system operates continuously with minimal power consumption and maintains data integrity even during network outages or power fluctuations.

### 2.2 Key Requirements

| Requirement Category | Specification |
|---------------------|---------------|
| **Data Collection** | Speed (CAN), Battery (ADC), GPS (UART) every 1-10 seconds |
| **Data Transmission** | Cellular (30s interval), LoRaWAN (5 min interval) |
| **Power Budget** | Average ≤ 10 mA, Deep sleep ≤ 5 µA |
| **Data Retention** | 72 hours of telemetry in non-volatile storage |
| **Operating Temp** | -40°C to +85°C (automotive grade) |
| **MTBF** | > 100,000 hours |
| **Latency** | Sensor sampling < 100 ms, Transmission < 5 s |

### 2.3 System Context Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                    Vehicle Environment                       │
│                                                              │
│  ┌──────────┐      ┌──────────┐      ┌──────────┐          │
│  │   CAN    │      │ Battery  │      │   GPS    │          │
│  │   Bus    │      │ Voltage  │      │ Module   │          │
│  └────┬─────┘      └────┬─────┘      └────┬─────┘          │
│       │                 │                   │                │
└───────┼─────────────────┼───────────────────┼────────────────┘
        │                 │                   │
        └─────────────────┼───────────────────┘
                          │
                    ┌─────▼──────┐
                    │ Telematics │
                    │    Unit    │
                    │  Firmware  │
                    └─────┬──────┘
                          │
            ┌─────────────┴─────────────┐
            │                           │
     ┌──────▼────────┐         ┌───────▼────────┐
     │    Cellular   │         │    LoRaWAN     │
     │  (LTE-M/NB)   │         │   Transceiver  │
     └──────┬────────┘         └───────┬────────┘
            │                           │
            └─────────────┬─────────────┘
                          │
                    ┌─────▼──────┐
                    │   Backend  │
                    │   System   │
                    └────────────┘
```

---

## 3. Hardware Platform

### 3.1 Microcontroller Unit (MCU)

**Selected Platform**: STM32L476RG (STMicroelectronics)

**Justification**:
- Ultra-low-power ARM Cortex-M4F core (80 MHz)
- Rich peripheral set: CAN, SPI, UART, I2C, ADC
- 1 MB Flash for firmware + data logging
- 128 KB SRAM for buffering and processing
- Multiple low-power modes (< 5 µA in standby)
- Automotive-grade temperature range (-40°C to +125°C)
- Extensive ecosystem and tooling support

**Key Peripherals Used**:
- **CAN**: bxCAN controller for vehicle speed data
- **ADC**: 12-bit, 5 Msps for battery voltage monitoring
- **UART**: GPS module interface (NMEA protocol)
- **SPI**: LoRaWAN transceiver interface
- **UART/AT**: Cellular modem communication
- **RTC**: Real-time clock for timestamping and wake-up
- **Flash**: Internal for firmware, external SPI NOR for data logging

### 3.2 Communication Hardware

#### 3.2.1 Cellular Modem
- **Model**: Quectel BG96
- **Technology**: LTE Cat M1 / NB-IoT / EGPRS
- **Interface**: UART (AT commands)
- **Power**: 180 mA peak (TX), 8 mA idle
- **Features**: Embedded TCP/IP stack, TLS, GPS/GNSS

#### 3.2.2 LoRaWAN Transceiver
- **Model**: Semtech SX1276
- **Interface**: SPI
- **Power**: 120 mA peak (TX), < 1 µA sleep
- **Range**: Up to 15 km line-of-sight
- **Frequency**: 868 MHz (EU) / 915 MHz (US)

### 3.3 Sensor Interfaces

| Sensor Type | Interface | Signal Characteristics |
|-------------|-----------|------------------------|
| **Vehicle Speed** | CAN 2.0B | 500 kbps, Standard frame |
| **Battery Voltage** | ADC Channel 1 | 0-30V via voltage divider (1:10) |
| **GPS Module** | UART (9600 baud) | NMEA 0183 format ($GPGGA, $GPRMC) |

### 3.4 Memory Architecture

```
┌─────────────────────────────────────┐
│     STM32L476RG Internal Memory      │
│                                      │
│  ┌────────────────────────────────┐ │
│  │  Flash (1 MB)                  │ │
│  │  ├─ Bootloader (32 KB)         │ │
│  │  ├─ Application (512 KB)       │ │
│  │  ├─ Configuration (4 KB)       │ │
│  │  └─ Reserved (476 KB)          │ │
│  └────────────────────────────────┘ │
│                                      │
│  ┌────────────────────────────────┐ │
│  │  SRAM (128 KB)                 │ │
│  │  ├─ Stack (8 KB)               │ │
│  │  ├─ Heap (32 KB)               │ │
│  │  ├─ Telemetry Buffer (64 KB)   │ │
│  │  └─ System (24 KB)             │ │
│  └────────────────────────────────┘ │
└─────────────────────────────────────┘

┌─────────────────────────────────────┐
│   External SPI Flash (4 MB)         │
│   ├─ Telemetry Log (3.5 MB)        │
│   ├─ Error Log (256 KB)            │
│   └─ Firmware Update Buffer (256KB)│
└─────────────────────────────────────┘
```

---

## 4. Software Architecture

### 4.1 Layered Architecture

The firmware follows a modular, layered architecture to ensure maintainability, testability, and portability:

```
┌───────────────────────────────────────────────────────────┐
│                     Application Layer                      │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────┐   │
│  │   Control   │  │  Telemetry  │  │  Configuration  │   │
│  │    Logic    │  │  Management │  │    Manager      │   │
│  └─────────────┘  └─────────────┘  └─────────────────┘   │
└────────────────────────┬──────────────────────────────────┘
                         │
┌────────────────────────┴──────────────────────────────────┐
│                    Middleware Layer                        │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐ │
│  │  Power   │  │   Data   │  │  Comms   │  │  Fault   │ │
│  │  Manager │  │  Buffer  │  │  Stack   │  │ Tolerance│ │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘ │
└────────────────────────┬──────────────────────────────────┘
                         │
┌────────────────────────┴──────────────────────────────────┐
│                     Driver Layer                           │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐ │
│  │   CAN    │  │   ADC    │  │   GPS    │  │  Modem   │ │
│  │  Driver  │  │  Driver  │  │  Driver  │  │  Driver  │ │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘ │
└────────────────────────┬──────────────────────────────────┘
                         │
┌────────────────────────┴──────────────────────────────────┐
│          Hardware Abstraction Layer (HAL)                  │
│  ┌────────┐  ┌────────┐  ┌────────┐  ┌────────┐         │
│  │  GPIO  │  │  Timer │  │  DMA   │  │  IRQ   │         │
│  └────────┘  └────────┘  └────────┘  └────────┘         │
└────────────────────────┬──────────────────────────────────┘
                         │
┌────────────────────────┴──────────────────────────────────┐
│                        RTOS (FreeRTOS)                     │
│     Task Scheduler │ Semaphores │ Queues │ Timers         │
└────────────────────────────────────────────────────────────┘
```

### 4.2 Task Architecture (FreeRTOS)

| Task Name | Priority | Period | Stack Size | Description |
|-----------|----------|--------|------------|-------------|
| `SensorAcqTask` | HIGH (3) | 1000 ms | 2 KB | Reads sensors (CAN, ADC, GPS) |
| `DataProcessTask` | MEDIUM (2) | 500 ms | 3 KB | Processes and packages telemetry |
| `CommTxTask` | MEDIUM (2) | 30000 ms | 4 KB | Transmits data via cellular/LoRa |
| `PowerMgrTask` | LOW (1) | 5000 ms | 1 KB | Manages power states |
| `HealthMonTask` | LOW (1) | 10000 ms | 1 KB | Monitors system health |
| `IdleTask` | IDLE (0) | - | 512 B | FreeRTOS idle hook |

**Inter-Task Communication**:
- **Queues**: Telemetry data from sensor task to processing task
- **Semaphores**: Mutual exclusion for shared resources (SPI flash, UART)
- **Event Groups**: Synchronization for power state transitions

### 4.3 Module Overview

#### 4.3.1 Sensor Drivers
- **CAN Driver**: Implements bxCAN filtering, reception, and speed extraction
- **ADC Driver**: Continuous conversion with DMA, voltage scaling
- **GPS Driver**: UART reception, NMEA parsing ($GPGGA, $GPRMC)

#### 4.3.2 Data Management
- **Circular Buffer**: Lock-free FIFO for telemetry samples
- **Data Packager**: Compresses and serializes telemetry into transmission frames
- **Flash Logger**: Persists data to external SPI flash for offline storage

#### 4.3.3 Communication Stack
- **Cellular Manager**: AT command interface, MQTT client, TLS wrapper
- **LoRaWAN Stack**: MAC layer implementation (Class A)
- **Message Queue**: Prioritized queue with retry logic

#### 4.3.4 Power Management
- **State Machine**: Transitions between Active, Idle, Sleep modes
- **Peripheral Controller**: Gates power to unused peripherals
- **Wake Manager**: Configures RTC and external interrupts for wake-up

#### 4.3.5 Fault Tolerance
- **Watchdog Manager**: Independent watchdog (IWDG) with 2s timeout
- **Health Monitor**: Checks sensor validity, memory integrity, communication status
- **Error Logger**: Persists errors with timestamps to flash

---

## 5. Sensor Interface Layer

### 5.1 CAN Bus Interface (Vehicle Speed)

**Hardware Configuration**:
- CAN peripheral: bxCAN1
- Bitrate: 500 kbps
- Filter: Standard ID 0x200 (vehicle speed message)

**Message Format**:
```c
// CAN Frame: ID 0x200, DLC 8
// Byte 0-1: Speed (km/h * 100, big-endian)
// Example: 0x1388 = 5000 = 50.00 km/h
```

**Driver API**:
```c
typedef struct {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
} CANMessage_t;

// Initialize CAN peripheral
StatusCode_t CAN_Init(uint32_t bitrate);

// Register callback for specific CAN ID
StatusCode_t CAN_RegisterRxCallback(uint32_t id, CANRxCallback_t callback);

// Extract vehicle speed from CAN message
float CAN_ExtractSpeed(const CANMessage_t *msg);
```

**Fault Handling**:
- Bus-off detection with automatic recovery
- Message timeout detection (> 2 seconds)
- CRC error counting and reporting

### 5.2 ADC Interface (Battery Voltage)

**Hardware Configuration**:
- ADC1, Channel 1 (PA0)
- Resolution: 12-bit (0-4095)
- Sampling rate: 10 Hz
- DMA circular mode
- Voltage divider: 10:1 (30V → 3.0V)

**Conversion Formula**:
```
Vbat = (ADC_Value / 4095) * VREF * 10
     = (ADC_Value / 4095) * 3.3 * 10
```

**Driver API**:
```c
// Initialize ADC with DMA
StatusCode_t ADC_Init(void);

// Start continuous conversion
StatusCode_t ADC_StartConversion(void);

// Get latest battery voltage (in volts)
float ADC_GetBatteryVoltage(void);

// Register callback for voltage threshold
StatusCode_t ADC_RegisterThresholdCallback(float threshold, ADCCallback_t callback);
```

**Calibration**:
- Offset calibration at startup
- Two-point calibration (stored in flash configuration)
- Temperature compensation using internal temp sensor

### 5.3 GPS Interface (Position Data)

**Hardware Configuration**:
- UART2, 9600 baud, 8N1
- GPS module: u-blox NEO-M8N or compatible
- Update rate: 1 Hz
- NMEA sentences: $GPGGA, $GPRMC

**NMEA Parsing**:
```c
// $GPGGA: Global Positioning System Fix Data
// $GPRMC: Recommended Minimum Specific GPS/Transit Data

typedef struct {
    float latitude;      // Decimal degrees (-90 to +90)
    float longitude;     // Decimal degrees (-180 to +180)
    float altitude;      // Meters above sea level
    uint8_t satellites;  // Number of satellites in use
    uint8_t fix_quality; // 0=Invalid, 1=GPS, 2=DGPS
    uint32_t timestamp;  // UTC time (HHMMSS)
} GPSData_t;
```

**Driver API**:
```c
// Initialize GPS UART interface
StatusCode_t GPS_Init(uint32_t baudrate);

// Parse incoming NMEA sentence
StatusCode_t GPS_ParseNMEA(const char *sentence, GPSData_t *gps_data);

// Get latest valid GPS position
StatusCode_t GPS_GetPosition(GPSData_t *gps_data);

// Check if GPS has valid fix
bool GPS_HasValidFix(void);
```

**Fault Handling**:
- NMEA checksum validation
- Sentence timeout detection (> 3 seconds)
- Fix quality monitoring
- Satellite count tracking

---

## 6. Data Acquisition and Buffering

### 6.1 Telemetry Data Structure

```c
#define TELEMETRY_VERSION 1

typedef struct __attribute__((packed)) {
    uint32_t timestamp;      // RTC timestamp (Unix epoch)
    float speed;             // km/h
    float battery_voltage;   // Volts
    float latitude;          // Decimal degrees
    float longitude;         // Decimal degrees
    float altitude;          // Meters
    uint8_t gps_satellites;  // Number of satellites
    uint8_t gps_fix_quality; // GPS fix quality
    uint8_t flags;           // Status flags (bit field)
    uint8_t reserved;        // Reserved for future use
    uint16_t crc16;          // CRC-16/CCITT checksum
} TelemetryRecord_t;  // 32 bytes

// Status flags bit definitions
#define FLAG_GPS_VALID      (1 << 0)
#define FLAG_CAN_VALID      (1 << 1)
#define FLAG_ADC_VALID      (1 << 2)
#define FLAG_LOW_BATTERY    (1 << 3)
#define FLAG_MOTION_DETECTED (1 << 4)
```

### 6.2 Circular Buffer Design

**Buffer Configuration**:
- Location: SRAM (64 KB allocation)
- Capacity: 2048 records (64 KB / 32 bytes)
- Access: Multi-producer, single-consumer
- Synchronization: Atomic operations (lock-free)

**Implementation**:
```c
typedef struct {
    TelemetryRecord_t records[BUFFER_CAPACITY];
    volatile uint32_t write_index;
    volatile uint32_t read_index;
    volatile uint32_t count;
    SemaphoreHandle_t mutex;
} CircularBuffer_t;

// Push telemetry record (non-blocking)
StatusCode_t Buffer_Push(CircularBuffer_t *buf, const TelemetryRecord_t *record);

// Pop telemetry record (non-blocking)
StatusCode_t Buffer_Pop(CircularBuffer_t *buf, TelemetryRecord_t *record);

// Get buffer utilization
uint32_t Buffer_GetCount(const CircularBuffer_t *buf);

// Check if buffer is full
bool Buffer_IsFull(const CircularBuffer_t *buf);
```

**Overflow Handling**:
- **Strategy**: Overwrite oldest records (FIFO)
- **Notification**: Set overflow flag, increment overflow counter
- **Recovery**: Trigger immediate transmission to drain buffer

### 6.3 Data Persistence (Flash Logging)

**Flash Memory Layout**:
```
External SPI Flash (4 MB)
├─ Sector 0-27: Telemetry Log (3.5 MB = 114,688 records)
├─ Sector 28:   Error Log (128 KB)
├─ Sector 29:   Configuration Backup (64 KB)
└─ Sector 30-31: Firmware Update Buffer (256 KB)
```

**Logging Strategy**:
- Write records in batches (32 records = 1 KB page)
- Circular log with wear leveling
- Persist every 5 minutes or when buffer > 50% full
- Retention: 72 hours at 1 Hz sampling rate

**Flash API**:
```c
// Initialize external flash
StatusCode_t Flash_Init(void);

// Write telemetry batch to flash
StatusCode_t Flash_WriteTelemetry(const TelemetryRecord_t *records, uint32_t count);

// Read telemetry from flash
StatusCode_t Flash_ReadTelemetry(TelemetryRecord_t *records, uint32_t offset, uint32_t count);

// Erase oldest sector (wear leveling)
StatusCode_t Flash_EraseSector(uint32_t sector);
```

### 6.4 Data Compression

**Compression Strategy**: Differential encoding + LZ77

**Compression Ratios**:
- Typical: 3:1 (32 bytes → 10.7 bytes average)
- GPS stationary: 5:1
- Highway driving: 2.5:1

**Implementation**:
```c
// Compress batch of telemetry records
uint32_t Compress_Telemetry(const TelemetryRecord_t *input, uint32_t count,
                             uint8_t *output, uint32_t output_size);

// Decompress telemetry (for debugging/testing)
uint32_t Decompress_Telemetry(const uint8_t *input, uint32_t input_size,
                               TelemetryRecord_t *output, uint32_t max_count);
```

---

## 7. Power Management

### 7.1 Power State Machine

```
                    ┌─────────────┐
                    │   STARTUP   │
                    └──────┬──────┘
                           │
                           ▼
                    ┌─────────────┐
          ┌────────▶│   ACTIVE    │◀────────┐
          │         └──────┬──────┘         │
          │                │                │
          │  Motion/Data   │  Idle Timeout  │  Data Ready
          │                ▼                │
          │         ┌─────────────┐         │
          │         │    IDLE     │─────────┘
          │         └──────┬──────┘
          │                │
          │  No Activity   │  Wake Event
          │  (5 minutes)   │
          │                ▼
          │         ┌─────────────┐
          └─────────│ DEEP_SLEEP  │
                    └─────────────┘
```

### 7.2 Power Modes

| Mode | CPU | Peripherals Active | Avg Current | Wake Sources |
|------|-----|-------------------|-------------|--------------|
| **ACTIVE** | 80 MHz | CAN, ADC, GPS, UART | 45 mA | - |
| **IDLE** | Sleep | RTC, CAN (filter), ADC (DMA) | 8 mA | CAN msg, Timer, ADC threshold |
| **DEEP_SLEEP** | Standby | RTC only | 2.5 µA | RTC alarm, External INT |

### 7.3 Power Management API

```c
typedef enum {
    POWER_MODE_ACTIVE,
    POWER_MODE_IDLE,
    POWER_MODE_DEEP_SLEEP
} PowerMode_t;

// Initialize power manager
StatusCode_t Power_Init(void);

// Request power mode transition
StatusCode_t Power_SetMode(PowerMode_t mode);

// Register wake source
StatusCode_t Power_RegisterWakeSource(WakeSource_t source, WakeCallback_t callback);

// Configure idle timeout
StatusCode_t Power_SetIdleTimeout(uint32_t seconds);

// Get current power consumption estimate
float Power_GetCurrentConsumption(void);
```

### 7.4 Peripheral Power Gating

**Gating Strategy**:
```c
// Peripheral states in different power modes
typedef struct {
    bool can_enabled;
    bool gps_enabled;
    bool cellular_enabled;
    bool lorawan_enabled;
    bool flash_enabled;
} PeripheralState_t;

// ACTIVE mode: All peripherals on
// IDLE mode: GPS off, Cellular off, LoRa off, CAN RX only
// DEEP_SLEEP: All peripherals off except RTC
```

### 7.5 Wake-Up Strategy

**Wake Events**:
1. **RTC Alarm**: Periodic wake for sensor sampling (1 Hz)
2. **CAN Message**: High-priority vehicle event (e.g., ignition on)
3. **Motion Detector**: Accelerometer interrupt (future enhancement)
4. **Low Battery**: ADC threshold interrupt

**Wake Latency**:
- IDLE → ACTIVE: < 10 ms
- DEEP_SLEEP → ACTIVE: < 100 ms

---

## 8. Communication Protocol Layer

### 8.1 Communication Manager Architecture

```
┌────────────────────────────────────────────────┐
│          Communication Manager                  │
│  ┌──────────────────────────────────────────┐  │
│  │       Message Queue (Priority)           │  │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  │  │
│  │  │ HIGH    │  │ MEDIUM  │  │  LOW    │  │  │
│  │  │(Alerts) │  │(Telem.) │  │ (Logs)  │  │  │
│  │  └─────────┘  └─────────┘  └─────────┘  │  │
│  └──────────────────┬───────────────────────┘  │
│                     │                           │
│       ┌─────────────┴─────────────┐             │
│       │                           │             │
│  ┌────▼────────┐          ┌───────▼────────┐   │
│  │  Cellular   │          │    LoRaWAN     │   │
│  │   Manager   │          │    Manager     │   │
│  └────┬────────┘          └───────┬────────┘   │
│       │                           │             │
└───────┼───────────────────────────┼─────────────┘
        │                           │
   ┌────▼─────┐              ┌──────▼──────┐
   │ BG96     │              │   SX1276    │
   │ Modem    │              │ Transceiver │
   └──────────┘              └─────────────┘
```

### 8.2 Cellular Communication (Primary)

**Protocol Stack**:
- Transport: MQTT v3.1.1 over TLS 1.2
- Network: TCP/IP (LTE-M or NB-IoT)
- AT Command Interface: UART @ 115200 baud

**Connection Flow**:
```
1. Power on modem (BG96)
2. Wait for network registration (< 30s)
3. Activate PDP context
4. Configure SSL/TLS certificates
5. Connect to MQTT broker (mqtt.example.com:8883)
6. Subscribe to command topics
7. Publish telemetry data
```

**MQTT Topics**:
- Publish: `vehicles/{device_id}/telemetry`
- Subscribe: `vehicles/{device_id}/commands`
- Will topic: `vehicles/{device_id}/status` (LWT)

**Cellular Manager API**:
```c
// Initialize cellular modem
StatusCode_t Cellular_Init(void);

// Connect to network
StatusCode_t Cellular_Connect(uint32_t timeout_ms);

// Publish telemetry message
StatusCode_t Cellular_PublishTelemetry(const uint8_t *data, uint32_t length);

// Check connection status
bool Cellular_IsConnected(void);

// Disconnect and power down
StatusCode_t Cellular_Disconnect(void);
```

**Retry Logic**:
- Max retries: 3 attempts
- Backoff: Exponential (30s, 60s, 120s)
- Fallback: Switch to LoRaWAN after 3 failed attempts

### 8.3 LoRaWAN Communication (Backup/Low-Power)

**Configuration**:
- Class: A (lowest power)
- Region: EU868 or US915
- Data Rate: ADR enabled (SF7-SF12)
- Confirmed uplinks: Enabled for critical messages

**LoRaWAN Stack**: Semtech LoRaMac-node reference implementation

**Transmission Strategy**:
- Interval: 5 minutes (configurable)
- Payload: Compressed telemetry batch (max 51 bytes)
- Duty cycle: < 1% (EU regulations)

**LoRaWAN Manager API**:
```c
// Initialize LoRaWAN stack
StatusCode_t LoRaWAN_Init(const LoRaWANConfig_t *config);

// Join network (OTAA)
StatusCode_t LoRaWAN_Join(uint32_t timeout_ms);

// Send uplink message
StatusCode_t LoRaWAN_SendUplink(uint8_t port, const uint8_t *data, uint8_t length, bool confirmed);

// Check join status
bool LoRaWAN_IsJoined(void);
```

### 8.4 Message Queue and Prioritization

**Priority Levels**:
1. **HIGH**: Alerts (low battery, faults) - Immediate transmission
2. **MEDIUM**: Normal telemetry - 30s interval
3. **LOW**: Debug logs, diagnostics - 5 min interval

**Queue Implementation**:
```c
typedef struct {
    uint8_t priority;
    uint8_t data[MAX_MESSAGE_SIZE];
    uint16_t length;
    uint8_t retries;
    uint32_t timestamp;
} QueuedMessage_t;

// Enqueue message with priority
StatusCode_t MsgQueue_Enqueue(uint8_t priority, const uint8_t *data, uint16_t length);

// Dequeue highest priority message
StatusCode_t MsgQueue_Dequeue(QueuedMessage_t *msg);

// Get queue depth
uint32_t MsgQueue_GetDepth(uint8_t priority);
```

### 8.5 Data Encryption and Integrity

**Encryption**:
- Algorithm: AES-128-CBC
- Key Management: Pre-shared key (stored in protected flash)
- IV: Random, transmitted with each message

**Integrity**:
- Algorithm: HMAC-SHA256
- MAC: Appended to encrypted payload

**Payload Format**:
```
┌─────────────────────────────────────────────┐
│ Header (4B) │ IV (16B) │ Encrypted Data    │
├─────────────┼──────────┼───────────────────┤
│ Version     │ Random   │ Compressed        │
│ Length      │ IV       │ Telemetry + MAC   │
└─────────────┴──────────┴───────────────────┘
```

---

## 9. Fault Tolerance and Diagnostics

### 9.1 Watchdog System

**Independent Watchdog (IWDG)**:
- Timeout: 2 seconds
- Refresh: Every 1 second (from HealthMonTask)
- Reset: Automatic MCU reset on timeout

**Watchdog Manager**:
```c
// Initialize watchdog with timeout
StatusCode_t Watchdog_Init(uint32_t timeout_ms);

// Refresh watchdog (must be called periodically)
void Watchdog_Refresh(void);

// Get watchdog reset flag
bool Watchdog_WasResetSource(void);
```

### 9.2 Health Monitoring

**Monitored Parameters**:
| Parameter | Threshold | Action |
|-----------|-----------|--------|
| Battery Voltage | < 11.5V | Alert + reduce sampling rate |
| GPS Fix Loss | > 5 minutes | Alert + increase GPS priority |
| CAN Timeout | > 2 seconds | Alert + use last known speed |
| Flash Write Errors | > 10 errors | Alert + use RAM buffer only |
| Cellular Connection | > 3 failures | Switch to LoRaWAN |
| Temperature | > 80°C | Alert + thermal throttling |

**Health Check API**:
```c
typedef struct {
    bool gps_healthy;
    bool can_healthy;
    bool cellular_healthy;
    bool flash_healthy;
    float temperature;
    float battery_voltage;
} SystemHealth_t;

// Perform health check
StatusCode_t Health_CheckSystem(SystemHealth_t *health);

// Get component health status
bool Health_IsComponentHealthy(Component_t component);

// Register health alert callback
StatusCode_t Health_RegisterAlertCallback(HealthAlertCallback_t callback);
```

### 9.3 Error Logging

**Error Log Format**:
```c
typedef struct __attribute__((packed)) {
    uint32_t timestamp;
    uint8_t severity;      // INFO, WARNING, ERROR, CRITICAL
    uint8_t component;     // GPS, CAN, Cellular, etc.
    uint16_t error_code;
    char message[32];
} ErrorLog_t;  // 40 bytes
```

**Logging API**:
```c
// Log error with severity
void Error_Log(Severity_t severity, Component_t component, uint16_t code, const char *msg);

// Retrieve error logs
uint32_t Error_GetLogs(ErrorLog_t *logs, uint32_t max_count);

// Clear error logs
void Error_ClearLogs(void);
```

**Persistence**:
- Storage: External SPI flash (128 KB = 3276 entries)
- Circular log with oldest entry overwrite
- Transmitted periodically with telemetry

### 9.4 Graceful Degradation

**Degradation Strategies**:

1. **GPS Loss**: Use last known position, flag as stale
2. **CAN Timeout**: Use zero speed, flag as invalid
3. **Low Battery**: Reduce sampling rate (1 Hz → 0.1 Hz), disable cellular
4. **Cellular Failure**: Switch to LoRaWAN exclusively
5. **Flash Failure**: Use RAM buffer only, transmit more frequently

**Degradation State Machine**:
```
NORMAL → WARNING → DEGRADED → CRITICAL → SAFE_MODE
```

---

## 10. Security Architecture

### 10.1 Secure Boot

- **Boot Sequence**: ROM Bootloader → Firmware Signature Check → Application
- **Signature**: ECDSA P-256
- **Public Key**: Stored in write-protected flash

### 10.2 Secure Storage

**Protected Data**:
- Encryption keys (AES-128)
- Device certificates (X.509)
- Configuration credentials

**Storage**: STM32 Flash Read Protection Level 2 (RDP2)

### 10.3 Communication Security

**TLS Configuration** (Cellular):
- Protocol: TLS 1.2
- Cipher Suite: TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256
- Certificates: Mutual TLS (device + CA certificates)

**LoRaWAN Security**:
- Network Session Key (NwkSKey)
- Application Session Key (AppSKey)
- AES-128 encryption for all MAC payloads

### 10.4 Firmware Updates (OTA)

**Update Process**:
1. Receive update notification via MQTT
2. Download firmware chunks (4 KB blocks)
3. Verify each chunk (SHA-256)
4. Store in external flash update buffer
5. Verify complete image (ECDSA signature)
6. Install and reboot

**Rollback Protection**:
- Dual-bank flash configuration
- Version number verification
- Automatic rollback on boot failure

---

## 11. Performance Requirements

### 11.1 Timing Requirements

| Operation | Max Latency | Typical |
|-----------|-------------|---------|
| Sensor Sampling | 100 ms | 50 ms |
| Data Packaging | 50 ms | 20 ms |
| Cellular Transmission | 5 s | 2 s |
| LoRaWAN Transmission | 10 s | 3 s |
| Wake from Deep Sleep | 100 ms | 50 ms |
| Watchdog Refresh | 2 s | 1 s |

### 11.2 Memory Footprint

| Component | Flash | SRAM |
|-----------|-------|------|
| Application Code | 180 KB | 12 KB |
| FreeRTOS | 15 KB | 8 KB |
| HAL/Drivers | 80 KB | 4 KB |
| Crypto Libraries | 45 KB | 2 KB |
| Communication Stacks | 120 KB | 20 KB |
| **Total** | **440 KB** | **46 KB** |
| **Margin** | 560 KB (56%) | 82 KB (64%) |

### 11.3 Power Budget

**Daily Energy Consumption** (12V battery):
```
Active (10 min/hour):   45 mA × 10/60 = 7.5 mA avg
Idle (45 min/hour):     8 mA × 45/60 = 6.0 mA avg
Deep Sleep (5 min/hour): 0.0025 mA × 5/60 = 0.0002 mA avg
Transmission (peak):    180 mA × 1s/30s = 6.0 mA avg
──────────────────────────────────────────────────
Total Average Current:  19.5 mA

Daily Energy: 19.5 mA × 24 h = 468 mAh/day
Weekly Energy: 3.28 Ah/week
Monthly Energy: 14.04 Ah/month
```

**Battery Life** (60 Ah vehicle battery, 50% DoD):
- Telematics only: 30 Ah / 0.468 Ah/day = **64 days**

---

## 12. System State Machine

### 12.1 Top-Level State Machine

```
┌──────────┐
│  POWER   │
│   ON     │
└────┬─────┘
     │
     ▼
┌──────────┐     Initialization    ┌──────────┐
│   INIT   │─────────Fail─────────▶│  ERROR   │
└────┬─────┘                        └────┬─────┘
     │                                   │
     │Success                            │Reset
     ▼                                   │
┌──────────┐                             │
│   IDLE   │◀────────────────────────────┘
└────┬─────┘
     │
     │Timer / Event
     ▼
┌──────────┐
│ SAMPLING │
└────┬─────┘
     │
     │Data Ready
     ▼
┌──────────┐
│PROCESSING│
└────┬─────┘
     │
     │Buffer > Threshold
     ▼
┌──────────┐
│TRANSMIT  │
└────┬─────┘
     │
     │Complete
     ▼
┌──────────┐     Idle Timeout      ┌──────────┐
│   IDLE   │─────────────────────▶│   SLEEP  │
└──────────┘                       └────┬─────┘
     ▲                                  │
     │                                  │
     └────────────Wake Event────────────┘
```

### 12.2 State Descriptions

| State | Description | Duration | Exit Condition |
|-------|-------------|----------|----------------|
| **INIT** | System initialization, peripheral setup | 2-5 s | Success or timeout |
| **IDLE** | Low-power wait, RTC running | Variable | Timer or external event |
| **SAMPLING** | Active sensor reading | 50-100 ms | All sensors read |
| **PROCESSING** | Data packaging and buffering | 20-50 ms | Processing complete |
| **TRANSMIT** | Network transmission | 2-10 s | TX complete or failure |
| **SLEEP** | Deep sleep mode | > 5 min | RTC alarm or interrupt |
| **ERROR** | Fault recovery and diagnostics | Variable | Recovery or reset |

---

## 13. Appendices

### 13.1 Acronyms

- **ADC**: Analog-to-Digital Converter
- **CAN**: Controller Area Network
- **DMA**: Direct Memory Access
- **GPS**: Global Positioning System
- **HAL**: Hardware Abstraction Layer
- **IWDG**: Independent Watchdog
- **LTE-M**: Long Term Evolution - Machine Type Communication
- **MQTT**: Message Queuing Telemetry Transport
- **NB-IoT**: Narrowband Internet of Things
- **NMEA**: National Marine Electronics Association
- **OTA**: Over-The-Air
- **RTC**: Real-Time Clock
- **RTOS**: Real-Time Operating System
- **TLS**: Transport Layer Security

### 13.2 References

1. STM32L476xx Reference Manual (RM0351)
2. Quectel BG96 Hardware Design Manual
3. Semtech SX1276 Datasheet
4. LoRaWAN 1.0.3 Specification
5. MQTT Version 3.1.1 Specification
6. ISO 26262 Road Vehicles - Functional Safety
7. FreeRTOS Reference Manual

### 13.3 Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | 2025-11-18 | Embedded Systems Team | Initial release |

---

**End of Document**
