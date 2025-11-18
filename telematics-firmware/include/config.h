/**
 * @file config.h
 * @brief System-wide configuration parameters
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef CONFIG_H
#define CONFIG_H

/******************************************************************************
 * Hardware Configuration
 ******************************************************************************/

/* Microcontroller */
#define MCU_CLOCK_FREQ_HZ       80000000UL  ///< 80 MHz system clock
#define MCU_FLASH_SIZE_KB       1024        ///< 1 MB Flash
#define MCU_SRAM_SIZE_KB        128         ///< 128 KB SRAM

/* External Flash */
#define EXT_FLASH_SIZE_BYTES    (4 * 1024 * 1024)  ///< 4 MB SPI Flash
#define EXT_FLASH_PAGE_SIZE     256                 ///< 256 byte pages
#define EXT_FLASH_SECTOR_SIZE   (4 * 1024)          ///< 4 KB sectors

/******************************************************************************
 * Sensor Configuration
 ******************************************************************************/

/* CAN Bus */
#define CAN_BITRATE             500000      ///< 500 kbps
#define CAN_SPEED_MSG_ID        0x200       ///< CAN ID for vehicle speed
#define CAN_TIMEOUT_MS          2000        ///< CAN message timeout

/* ADC (Battery Voltage) */
#define ADC_RESOLUTION_BITS     12          ///< 12-bit ADC
#define ADC_VREF_MV             3300        ///< 3.3V reference voltage
#define ADC_VOLTAGE_DIVIDER     10.0f       ///< 10:1 voltage divider
#define ADC_SAMPLING_RATE_HZ    10          ///< 10 Hz sampling rate
#define BATTERY_LOW_THRESHOLD_V 11.5f       ///< Low battery threshold

/* GPS Module */
#define GPS_UART_BAUDRATE       9600        ///< GPS module baud rate
#define GPS_UPDATE_RATE_HZ      1           ///< 1 Hz GPS update rate
#define GPS_TIMEOUT_MS          3000        ///< GPS data timeout
#define GPS_MIN_SATELLITES      4           ///< Minimum satellites for valid fix

/******************************************************************************
 * Data Buffering Configuration
 ******************************************************************************/

#define TELEMETRY_BUFFER_SIZE   2048        ///< Circular buffer capacity (records)
#define TELEMETRY_RECORD_SIZE   32          ///< Size of one telemetry record (bytes)
#define BUFFER_SIZE_BYTES       (TELEMETRY_BUFFER_SIZE * TELEMETRY_RECORD_SIZE)

#define FLASH_LOG_SIZE_RECORDS  114688      ///< Flash log capacity (3.5 MB)
#define FLASH_LOG_BATCH_SIZE    32          ///< Records per flash write batch

/******************************************************************************
 * Power Management Configuration
 ******************************************************************************/

/* Power Mode Current Consumption (mA) */
#define CURRENT_ACTIVE_MA       45.0f
#define CURRENT_IDLE_MA         8.0f
#define CURRENT_DEEP_SLEEP_UA   2.5f

/* Power Mode Timeouts */
#define IDLE_TIMEOUT_MS         30000       ///< 30 seconds before IDLE mode
#define SLEEP_TIMEOUT_MS        300000      ///< 5 minutes before DEEP_SLEEP

/* Wake-up Configuration */
#define RTC_WAKEUP_INTERVAL_MS  1000        ///< 1 second RTC wake interval

/******************************************************************************
 * Communication Configuration
 ******************************************************************************/

/* Cellular Modem */
#define CELLULAR_UART_BAUDRATE  115200      ///< Modem UART baud rate
#define CELLULAR_TX_INTERVAL_MS 30000       ///< 30 second transmission interval
#define CELLULAR_CONNECT_TIMEOUT_MS 30000   ///< 30 second connection timeout
#define CELLULAR_MAX_RETRIES    3           ///< Maximum connection retries

/* LoRaWAN */
#define LORAWAN_TX_INTERVAL_MS  300000      ///< 5 minute transmission interval
#define LORAWAN_MAX_PAYLOAD     51          ///< Maximum LoRaWAN payload (bytes)
#define LORAWAN_JOIN_TIMEOUT_MS 60000       ///< 60 second join timeout

/* MQTT Configuration */
#define MQTT_BROKER_HOST        "mqtt.example.com"
#define MQTT_BROKER_PORT        8883        ///< MQTT over TLS
#define MQTT_KEEPALIVE_SEC      60
#define MQTT_QOS_LEVEL          1           ///< At least once delivery

/* Message Queue */
#define MSG_QUEUE_DEPTH_HIGH    10          ///< High priority queue depth
#define MSG_QUEUE_DEPTH_MEDIUM  50          ///< Medium priority queue depth
#define MSG_QUEUE_DEPTH_LOW     20          ///< Low priority queue depth

/******************************************************************************
 * Compression and Encryption
 ******************************************************************************/

#define ENABLE_COMPRESSION      1           ///< Enable data compression
#define ENABLE_ENCRYPTION       1           ///< Enable data encryption

#define AES_KEY_SIZE_BITS       128         ///< AES-128 encryption
#define AES_BLOCK_SIZE          16          ///< 16-byte AES blocks

/******************************************************************************
 * Fault Tolerance Configuration
 ******************************************************************************/

/* Watchdog */
#define WATCHDOG_TIMEOUT_MS     2000        ///< 2 second watchdog timeout
#define WATCHDOG_REFRESH_MS     1000        ///< 1 second refresh interval

/* Health Monitoring */
#define HEALTH_CHECK_INTERVAL_MS 10000      ///< 10 second health check interval
#define MAX_CONSECUTIVE_ERRORS   10         ///< Max errors before degradation

/* Error Logging */
#define ERROR_LOG_SIZE_ENTRIES  3276        ///< Error log capacity (128 KB)

/* Temperature Limits */
#define TEMP_WARNING_CELSIUS    70.0f       ///< Warning temperature
#define TEMP_CRITICAL_CELSIUS   80.0f       ///< Critical temperature

/******************************************************************************
 * FreeRTOS Configuration
 ******************************************************************************/

#define configCPU_CLOCK_HZ              MCU_CLOCK_FREQ_HZ
#define configTICK_RATE_HZ              1000    ///< 1 ms tick
#define configMAX_PRIORITIES            5
#define configMINIMAL_STACK_SIZE        128     ///< Words (512 bytes)
#define configTOTAL_HEAP_SIZE           (32 * 1024)  ///< 32 KB heap

/* Task Priorities */
#define PRIORITY_SENSOR_ACQ     3           ///< HIGH
#define PRIORITY_DATA_PROCESS   2           ///< MEDIUM
#define PRIORITY_COMM_TX        2           ///< MEDIUM
#define PRIORITY_POWER_MGR      1           ///< LOW
#define PRIORITY_HEALTH_MON     1           ///< LOW

/* Task Stack Sizes (bytes) */
#define STACK_SIZE_SENSOR_ACQ   (2 * 1024)
#define STACK_SIZE_DATA_PROCESS (3 * 1024)
#define STACK_SIZE_COMM_TX      (4 * 1024)
#define STACK_SIZE_POWER_MGR    (1 * 1024)
#define STACK_SIZE_HEALTH_MON   (1 * 1024)

/******************************************************************************
 * Debug Configuration
 ******************************************************************************/

#define DEBUG_ENABLE            1           ///< Enable debug features
#define DEBUG_UART_BAUDRATE     115200      ///< Debug UART baud rate

#if DEBUG_ENABLE
    #define DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)
#endif

/******************************************************************************
 * Version Information
 ******************************************************************************/

#define FIRMWARE_VERSION_MAJOR  1
#define FIRMWARE_VERSION_MINOR  0
#define FIRMWARE_VERSION_PATCH  0
#define FIRMWARE_BUILD_DATE     "2025-11-18"

#endif /* CONFIG_H */
