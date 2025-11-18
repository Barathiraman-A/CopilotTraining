# Vehicle Telematics Unit - Detailed Design Specification

**Document Version:** 1.0  
**Date:** November 18, 2025  
**Author:** Embedded Systems Team

---

## Table of Contents

1. [Introduction](#introduction)
2. [Component Specifications](#component-specifications)
3. [API Reference](#api-reference)
4. [Data Flow Diagrams](#data-flow-diagrams)
5. [Memory Maps](#memory-maps)
6. [Timing Analysis](#timing-analysis)
7. [Test Specifications](#test-specifications)

---

## 1. Introduction

This document provides detailed design specifications for all firmware components of the vehicle telematics unit. It serves as a reference for developers implementing, testing, and maintaining the system.

### 1.1 Document Purpose

- Detailed component specifications
- Complete API reference documentation
- Implementation guidelines
- Testing requirements

### 1.2 Related Documents

- `architecture.md` - High-level firmware architecture
- `integration_guide.md` - Integration and deployment procedures
- `README.md` - Project overview and quick start

---

## 2. Component Specifications

### 2.1 CAN Driver (`can_driver.c`)

**Purpose**: Interface with vehicle CAN bus to extract speed data.

**Dependencies**:
- STM32 HAL bxCAN peripheral driver
- System clock configuration
- NVIC configuration

**Key Features**:
- Message filtering for ID 0x200 (speed)
- Automatic bus-off recovery
- Error statistics tracking
- Health monitoring with timeout detection

**Configuration Parameters**:
```c
#define CAN_BITRATE         500000      // 500 kbps
#define CAN_SPEED_MSG_ID    0x200       // Speed message ID
#define CAN_TIMEOUT_MS      2000        // Message timeout
#define CAN_MAX_ERRORS      100         // Error threshold
```

**State Machine**:
```
UNINITIALIZED → INITIALIZED → RUNNING
     ↓              ↓             ↓
   ERROR ← ← ← ← ← ← ← ← ← ← ← ERROR
```

**Error Handling**:
- Bus-off: Automatic recovery after 128 idle frames
- CRC errors: Increment error counter, log to flash
- Timeout: Flag as unhealthy, use last known speed

**Testing Requirements**:
1. Unit tests for speed extraction algorithm
2. Integration tests with CAN simulator
3. Bus-off recovery verification
4. Error injection testing

---

### 2.2 ADC Driver (`adc_driver.c`)

**Purpose**: Monitor battery voltage via 12-bit ADC with DMA.

**Dependencies**:
- STM32 HAL ADC peripheral driver
- DMA controller
- GPIO configuration (PA0)

**Key Features**:
- Continuous conversion with DMA circular buffer
- Averaging for noise reduction (16 samples)
- Threshold monitoring with callbacks
- Two-point calibration support

**Conversion Formula**:
```
Vbat = (ADC_Raw / 4095) × 3.3V × 10 × scale_factor + offset
```

**Calibration Procedure**:
1. Measure actual battery voltage with external meter
2. Read raw ADC value
3. Calculate scale factor: `scale = Vactual / Vuncalibrated`
4. Store scale factor to flash

**Noise Mitigation**:
- Hardware: RC filter (10kΩ, 100nF) on ADC input
- Software: Moving average of 16 samples
- Sampling: 10 Hz with oversampling

**Testing Requirements**:
1. Linearity test across 9-30V range
2. Accuracy verification (±0.1V)
3. Threshold callback testing
4. DMA overflow handling

---

### 2.3 GPS Driver (`gps_driver.c`)

**Purpose**: Parse NMEA sentences from GPS module to extract position data.

**Dependencies**:
- UART2 peripheral (9600 baud, 8N1)
- RX interrupt
- String parsing functions

**Supported NMEA Sentences**:
- `$GPGGA`: Global Positioning System Fix Data
- `$GPRMC`: Recommended Minimum Specific GPS/Transit Data
- `$GNGGA`: GNSS Fix Data (multi-constellation)
- `$GNRMC`: GNSS Recommended Minimum

**Parsing Flow**:
```
UART RX Interrupt → Accumulate Characters → Detect '\n' 
    → Validate Checksum → Parse Sentence → Update Position
    → Invoke Callback
```

**Coordinate Conversion**:
NMEA format (DDMM.MMMM) is converted to decimal degrees:
```
Decimal Degrees = DD + (MM.MMMM / 60)
```

**Health Criteria**:
- Valid fix within last 3 seconds
- Minimum 4 satellites
- Fix quality > 0
- HDOP < 5.0

**Testing Requirements**:
1. NMEA sentence parser unit tests
2. Checksum validation tests
3. Coordinate conversion accuracy
4. Timeout and recovery testing

---

### 2.4 Circular Buffer (`circular_buffer.c`)

**Purpose**: Lock-free FIFO buffer for telemetry records.

**Algorithm**: Lock-free using atomic operations
- `__atomic_fetch_add()` for indices
- `__atomic_load_n()` for reads
- `__atomic_store_n()` for writes

**Capacity**: 2048 records (64 KB)

**Overflow Behavior**: Overwrite oldest records (FIFO)

**Thread Safety**: Yes (multi-producer, single-consumer)

**Performance**:
- Push: O(1), ~50 cycles
- Pop: O(1), ~50 cycles
- Peek: O(1), ~30 cycles

**Memory Layout**:
```
┌──────────────────────────────────────────┐
│  Record 0 (32 bytes)                     │
├──────────────────────────────────────────┤
│  Record 1 (32 bytes)                     │
├──────────────────────────────────────────┤
│  ...                                     │
├──────────────────────────────────────────┤
│  Record 2047 (32 bytes)                  │
└──────────────────────────────────────────┘
   write_index ────┐  ┌──── read_index
                   ↓  ↓
```

**Testing Requirements**:
1. Concurrent push/pop stress test
2. Overflow handling verification
3. Boundary condition testing
4. Memory corruption detection

---

### 2.5 Power Manager (`power_manager.c`)

**Purpose**: Manage system power states for optimal energy consumption.

**Power Modes**:

| Mode | CPU State | Peripherals | Current | Wake Latency |
|------|-----------|-------------|---------|--------------|
| ACTIVE | 80 MHz | All on | 45 mA | - |
| IDLE | Sleep | Essential | 8 mA | < 10 ms |
| DEEP_SLEEP | Standby | RTC only | 2.5 µA | < 100 ms |

**State Transition Logic**:
```c
if (no_activity_for > 30s) {
    transition_to(IDLE);
}
if (no_activity_for > 5min && buffer_empty) {
    transition_to(DEEP_SLEEP);
}
```

**Energy Accounting**:
```c
Energy (mAh) = Σ (Current_i × Time_i) / 3600
```

**Peripheral Power Gating**:
- CAN: Sleep mode command
- GPS: PMTK command for standby
- Cellular: AT+QSCLK=1 (sleep mode)
- Flash: Deep power-down command

**Testing Requirements**:
1. Current consumption measurement per mode
2. Transition timing verification
3. Wake source testing
4. Energy accounting accuracy

---

## 3. API Reference

### 3.1 CAN Driver API

#### `CAN_Init()`
```c
StatusCode_t CAN_Init(uint32_t bitrate);
```
**Description**: Initializes CAN peripheral with specified bitrate.

**Parameters**:
- `bitrate`: CAN bus bitrate in bps (e.g., 500000)

**Returns**: `STATUS_OK` on success, error code otherwise

**Example**:
```c
if (CAN_Init(500000) != STATUS_OK) {
    // Handle error
}
```

#### `CAN_RegisterRxCallback()`
```c
StatusCode_t CAN_RegisterRxCallback(uint32_t id, CANRxCallback_t callback);
```
**Description**: Registers callback for specific CAN message ID.

**Parameters**:
- `id`: CAN message identifier (e.g., 0x200 for speed)
- `callback`: Function pointer to invoke on message reception

**Returns**: `STATUS_OK` on success

**Example**:
```c
void SpeedCallback(const CANMessage_t *msg) {
    float speed = CAN_ExtractSpeed(msg);
    // Process speed data
}

CAN_RegisterRxCallback(0x200, SpeedCallback);
```

---

### 3.2 GPS Driver API

#### `GPS_GetPosition()`
```c
StatusCode_t GPS_GetPosition(GPSData_t *gps_data);
```
**Description**: Retrieves latest valid GPS position.

**Parameters**:
- `gps_data`: Pointer to structure to populate with GPS data

**Returns**:
- `STATUS_OK`: Valid GPS data available
- `STATUS_NO_DATA`: No valid fix
- `STATUS_NOT_INITIALIZED`: GPS not initialized

**Example**:
```c
GPSData_t gps;
if (GPS_GetPosition(&gps) == STATUS_OK) {
    printf("Lat: %.6f, Lon: %.6f\n", gps.latitude, gps.longitude);
}
```

---

### 3.3 Circular Buffer API

#### `CircularBuffer_Push()`
```c
StatusCode_t CircularBuffer_Push(CircularBuffer_t *buffer, 
                                  const TelemetryRecord_t *record);
```
**Description**: Pushes telemetry record into circular buffer.

**Parameters**:
- `buffer`: Pointer to circular buffer
- `record`: Pointer to telemetry record to push

**Returns**:
- `STATUS_OK`: Record pushed successfully
- `STATUS_BUFFER_FULL`: Buffer full (oldest record overwritten)

**Thread Safety**: Yes (lock-free atomic operations)

**Example**:
```c
TelemetryRecord_t record;
// Populate record...
CircularBuffer_Push(&buffer, &record);
```

---

## 4. Data Flow Diagrams

### 4.1 Sensor Data Acquisition Flow

```
┌──────────┐        ┌──────────┐        ┌──────────┐
│   CAN    │        │   ADC    │        │   GPS    │
│  Module  │        │  Module  │        │  Module  │
└────┬─────┘        └────┬─────┘        └────┬─────┘
     │                   │                    │
     │ CAN Frame         │ Voltage            │ NMEA
     │ (500 kbps)        │ (10 Hz)            │ (1 Hz)
     │                   │                    │
     ▼                   ▼                    ▼
┌────────────────────────────────────────────────┐
│         Sensor Acquisition Task                │
│  - Extract speed from CAN                      │
│  - Read battery voltage                        │
│  - Parse GPS coordinates                       │
│  - Create telemetry record                     │
│  - Calculate CRC16                             │
└────────────────────┬───────────────────────────┘
                     │
                     ▼
          ┌──────────────────┐
          │ Circular Buffer  │
          │  (2048 records)  │
          └──────────────────┘
```

### 4.2 Data Transmission Flow

```
┌──────────────────┐
│ Circular Buffer  │
└────────┬─────────┘
         │
         │ Pop Batch (32 records)
         ▼
┌─────────────────────┐
│ Data Processing     │
│ - Compression (3:1) │
│ - Encryption (AES)  │
│ - Packaging         │
└────────┬────────────┘
         │
         ▼
┌─────────────────────┐
│  Message Queue      │
│  Priority: H/M/L    │
└────────┬────────────┘
         │
    ┌────┴────┐
    │         │
    ▼         ▼
┌─────┐   ┌────────┐
│Cell │   │LoRaWAN │
│ular │   │        │
└─────┘   └────────┘
```

---

## 5. Memory Maps

### 5.1 SRAM Allocation (128 KB)

| Region | Start | End | Size | Purpose |
|--------|-------|-----|------|---------|
| Stack | 0x20000000 | 0x20002000 | 8 KB | Task stacks |
| Heap | 0x20002000 | 0x2000A000 | 32 KB | Dynamic allocation |
| Buffer | 0x2000A000 | 0x2001A000 | 64 KB | Telemetry circular buffer |
| System | 0x2001A000 | 0x20020000 | 24 KB | Global variables, BSS |

### 5.2 Flash Allocation (1 MB)

| Region | Start | End | Size | Purpose |
|--------|-------|-----|------|---------|
| Bootloader | 0x08000000 | 0x08008000 | 32 KB | Secure bootloader |
| Application | 0x08008000 | 0x08088000 | 512 KB | Firmware code |
| Configuration | 0x08088000 | 0x08089000 | 4 KB | System configuration |
| Reserved | 0x08089000 | 0x08100000 | 476 KB | Future use |

---

## 6. Timing Analysis

### 6.1 Task Execution Times

| Task | WCET | Typical | Frequency |
|------|------|---------|-----------|
| SensorAcquisition | 80 ms | 50 ms | 1 Hz |
| DataProcessing | 40 ms | 20 ms | 2 Hz |
| CommTx | 4.5 s | 2 s | 0.033 Hz (30s) |
| PowerMgr | 5 ms | 2 ms | 0.2 Hz (5s) |
| HealthMonitor | 15 ms | 8 ms | 0.1 Hz (10s) |

**WCET**: Worst Case Execution Time

### 6.2 ISR Latency Budget

| ISR | Priority | Max Latency | Purpose |
|-----|----------|-------------|---------|
| CAN RX | 5 (highest) | 10 µs | CAN frame reception |
| UART RX (GPS) | 4 | 100 µs | GPS byte reception |
| ADC DMA | 3 | 500 µs | ADC conversion complete |
| RTC Alarm | 2 | 1 ms | Power management wake |
| SysTick | 1 | 100 µs | FreeRTOS tick |

---

## 7. Test Specifications

### 7.1 Unit Test Requirements

**CAN Driver Tests**:
- [ ] Speed extraction accuracy (±0.1 km/h)
- [ ] Message filtering (100% correct ID filtering)
- [ ] Error counter increment on CRC errors
- [ ] Timeout detection (< 10 ms detection latency)

**ADC Driver Tests**:
- [ ] Voltage linearity (R² > 0.999)
- [ ] Averaging algorithm (< 1% noise)
- [ ] Threshold callback invocation
- [ ] Calibration persistence

**GPS Driver Tests**:
- [ ] NMEA checksum validation (100% detection)
- [ ] Coordinate conversion accuracy (< 1m error)
- [ ] Sentence parsing robustness
- [ ] Fix loss detection (< 5s latency)

**Circular Buffer Tests**:
- [ ] Lock-free correctness (race condition free)
- [ ] FIFO ordering maintained
- [ ] Overflow handling (no data corruption)
- [ ] Concurrent access stress test (1000 ops/s)

### 7.2 Integration Test Requirements

**End-to-End Data Flow**:
- [ ] Sensor → Buffer → Transmission pipeline
- [ ] Data integrity (CRC verification)
- [ ] Latency measurement (< 5s end-to-end)

**Power Management**:
- [ ] Mode transitions functional
- [ ] Current consumption verification (±10%)
- [ ] Wake-up latency (< spec)

**Fault Injection**:
- [ ] CAN bus-off recovery
- [ ] GPS loss graceful degradation
- [ ] Network failure fallback
- [ ] Watchdog reset recovery

### 7.3 Acceptance Criteria

| Requirement | Target | Test Method |
|-------------|--------|-------------|
| Power consumption (avg) | < 10 mA | Power analyzer |
| GPS accuracy | < 5 m | Known location test |
| CAN latency | < 100 ms | CAN analyzer |
| Data retention | 72 hours | Flash read-back test |
| MTBF | > 100,000 hours | Accelerated life test |
| Operating temp | -40°C to +85°C | Thermal chamber |

---

**End of Document**
