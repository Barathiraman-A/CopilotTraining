/**
 * @file telemetry_types.h
 * @brief Common data types and structures for vehicle telematics system
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef TELEMETRY_TYPES_H
#define TELEMETRY_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/******************************************************************************
 * Status Codes
 ******************************************************************************/

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_TIMEOUT = 2,
    STATUS_BUSY = 3,
    STATUS_INVALID_PARAM = 4,
    STATUS_NO_DATA = 5,
    STATUS_BUFFER_FULL = 6,
    STATUS_NOT_INITIALIZED = 7,
    STATUS_HARDWARE_ERROR = 8
} StatusCode_t;

/******************************************************************************
 * Telemetry Data Structures
 ******************************************************************************/

#define TELEMETRY_VERSION 1

/**
 * @brief Main telemetry record structure (32 bytes, packed)
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;      ///< RTC timestamp (Unix epoch seconds)
    float speed;             ///< Vehicle speed in km/h
    float battery_voltage;   ///< Battery voltage in volts
    float latitude;          ///< GPS latitude in decimal degrees
    float longitude;         ///< GPS longitude in decimal degrees
    float altitude;          ///< GPS altitude in meters above sea level
    uint8_t gps_satellites;  ///< Number of satellites in use
    uint8_t gps_fix_quality; ///< GPS fix quality (0=Invalid, 1=GPS, 2=DGPS)
    uint8_t flags;           ///< Status flags (bit field)
    uint8_t reserved;        ///< Reserved for future use
    uint16_t crc16;          ///< CRC-16/CCITT checksum
} TelemetryRecord_t;

/**
 * @brief Telemetry status flags (bit definitions)
 */
#define FLAG_GPS_VALID       (1 << 0)  ///< GPS data is valid
#define FLAG_CAN_VALID       (1 << 1)  ///< CAN speed data is valid
#define FLAG_ADC_VALID       (1 << 2)  ///< ADC battery voltage is valid
#define FLAG_LOW_BATTERY     (1 << 3)  ///< Battery voltage below threshold
#define FLAG_MOTION_DETECTED (1 << 4)  ///< Vehicle motion detected
#define FLAG_DATA_COMPRESSED (1 << 5)  ///< Data has been compressed
#define FLAG_FAULT_PRESENT   (1 << 6)  ///< System fault detected
#define FLAG_NETWORK_ERROR   (1 << 7)  ///< Network communication error

/******************************************************************************
 * GPS Data Structures
 ******************************************************************************/

/**
 * @brief GPS position data
 */
typedef struct {
    float latitude;      ///< Decimal degrees (-90 to +90)
    float longitude;     ///< Decimal degrees (-180 to +180)
    float altitude;      ///< Meters above sea level
    uint8_t satellites;  ///< Number of satellites in use
    uint8_t fix_quality; ///< 0=Invalid, 1=GPS, 2=DGPS, 3=PPS
    uint32_t timestamp;  ///< UTC time (HHMMSS format)
    uint16_t hdop;       ///< Horizontal dilution of precision * 100
    bool valid;          ///< True if fix is valid
} GPSData_t;

/******************************************************************************
 * CAN Data Structures
 ******************************************************************************/

/**
 * @brief CAN message structure
 */
typedef struct {
    uint32_t id;         ///< CAN message identifier
    uint8_t data[8];     ///< CAN data payload (max 8 bytes)
    uint8_t dlc;         ///< Data length code (0-8)
    bool is_extended;    ///< Extended frame format flag
    uint32_t timestamp;  ///< Reception timestamp
} CANMessage_t;

/******************************************************************************
 * System Health Structures
 ******************************************************************************/

/**
 * @brief Component identifiers for health monitoring
 */
typedef enum {
    COMPONENT_GPS = 0,
    COMPONENT_CAN = 1,
    COMPONENT_ADC = 2,
    COMPONENT_CELLULAR = 3,
    COMPONENT_LORAWAN = 4,
    COMPONENT_FLASH = 5,
    COMPONENT_POWER = 6,
    COMPONENT_WATCHDOG = 7,
    COMPONENT_COUNT = 8
} Component_t;

/**
 * @brief System health status
 */
typedef struct {
    bool gps_healthy;
    bool can_healthy;
    bool cellular_healthy;
    bool flash_healthy;
    float temperature;       ///< MCU temperature in Celsius
    float battery_voltage;   ///< Current battery voltage
    uint32_t uptime_seconds; ///< System uptime
    uint32_t error_count;    ///< Total error count
} SystemHealth_t;

/******************************************************************************
 * Error Logging Structures
 ******************************************************************************/

/**
 * @brief Error severity levels
 */
typedef enum {
    SEVERITY_INFO = 0,
    SEVERITY_WARNING = 1,
    SEVERITY_ERROR = 2,
    SEVERITY_CRITICAL = 3
} Severity_t;

/**
 * @brief Error log entry (40 bytes, packed)
 */
typedef struct __attribute__((packed)) {
    uint32_t timestamp;      ///< Error timestamp (Unix epoch)
    uint8_t severity;        ///< Severity level
    uint8_t component;       ///< Component identifier
    uint16_t error_code;     ///< Component-specific error code
    char message[32];        ///< Human-readable error message
} ErrorLog_t;

/******************************************************************************
 * Power Management Structures
 ******************************************************************************/

/**
 * @brief Power management modes
 */
typedef enum {
    POWER_MODE_ACTIVE,      ///< All systems active (45 mA)
    POWER_MODE_IDLE,        ///< CPU sleep, peripherals on (8 mA)
    POWER_MODE_DEEP_SLEEP   ///< Standby mode, RTC only (2.5 µA)
} PowerMode_t;

/**
 * @brief Wake source identifiers
 */
typedef enum {
    WAKE_SOURCE_RTC_ALARM = 0,
    WAKE_SOURCE_CAN_MESSAGE = 1,
    WAKE_SOURCE_EXTERNAL_INT = 2,
    WAKE_SOURCE_ADC_THRESHOLD = 3,
    WAKE_SOURCE_MOTION = 4
} WakeSource_t;

/******************************************************************************
 * Communication Structures
 ******************************************************************************/

/**
 * @brief Message priority levels
 */
typedef enum {
    PRIORITY_HIGH = 0,      ///< Alerts, immediate transmission
    PRIORITY_MEDIUM = 1,    ///< Normal telemetry (30s interval)
    PRIORITY_LOW = 2        ///< Diagnostics, logs (5 min interval)
} MessagePriority_t;

/**
 * @brief Queued message structure
 */
typedef struct {
    uint8_t priority;
    uint8_t data[256];
    uint16_t length;
    uint8_t retries;
    uint32_t timestamp;
} QueuedMessage_t;

/**
 * @brief Communication channel identifiers
 */
typedef enum {
    COMM_CHANNEL_CELLULAR = 0,
    COMM_CHANNEL_LORAWAN = 1,
    COMM_CHANNEL_NONE = 255
} CommChannel_t;

/******************************************************************************
 * Configuration Structures
 ******************************************************************************/

/**
 * @brief System configuration parameters
 */
typedef struct {
    uint32_t sampling_interval_ms;   ///< Sensor sampling interval
    uint32_t tx_interval_cellular_ms; ///< Cellular transmission interval
    uint32_t tx_interval_lorawan_ms;  ///< LoRaWAN transmission interval
    float battery_threshold_v;        ///< Low battery threshold
    uint8_t max_retries;              ///< Maximum transmission retries
    bool enable_compression;          ///< Enable data compression
    bool enable_encryption;           ///< Enable data encryption
    PowerMode_t default_power_mode;   ///< Default power mode
} SystemConfig_t;

/******************************************************************************
 * Utility Macros
 ******************************************************************************/

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, low, high) (MIN(MAX((x), (low)), (high)))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define UNUSED(x) (void)(x)

/**
 * @brief Convert km/h to m/s
 */
#define KMH_TO_MS(kmh) ((kmh) * 0.277778f)

/**
 * @brief Convert m/s to km/h
 */
#define MS_TO_KMH(ms) ((ms) * 3.6f)

#endif /* TELEMETRY_TYPES_H */
