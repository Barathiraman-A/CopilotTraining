/**
 * @file gps_driver.h
 * @brief GPS module driver for position data acquisition
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef GPS_DRIVER_H
#define GPS_DRIVER_H

#include "telemetry_types.h"
#include "config.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief GPS data ready callback function type
 * @param gps_data Pointer to valid GPS data
 */
typedef void (*GPSDataCallback_t)(const GPSData_t *gps_data);

/******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize GPS module UART interface
 * @param baudrate UART baud rate (typically 9600)
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t GPS_Init(uint32_t baudrate);

/**
 * @brief De-initialize GPS module
 * @return STATUS_OK on success
 */
StatusCode_t GPS_Deinit(void);

/**
 * @brief Parse NMEA sentence and extract GPS data
 * @param sentence NULL-terminated NMEA sentence string
 * @param gps_data Pointer to GPS data structure to populate
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t GPS_ParseNMEA(const char *sentence, GPSData_t *gps_data);

/**
 * @brief Get latest valid GPS position
 * @param gps_data Pointer to GPS data structure to populate
 * @return STATUS_OK if valid data available, STATUS_NO_DATA otherwise
 */
StatusCode_t GPS_GetPosition(GPSData_t *gps_data);

/**
 * @brief Check if GPS has valid fix
 * @return True if valid fix, false otherwise
 */
bool GPS_HasValidFix(void);

/**
 * @brief Register callback for GPS data updates
 * @param callback Function to call when new GPS data is available
 * @return STATUS_OK on success
 */
StatusCode_t GPS_RegisterCallback(GPSDataCallback_t callback);

/**
 * @brief Check if GPS module is healthy
 * @return True if healthy, false otherwise
 */
bool GPS_IsHealthy(void);

/**
 * @brief Get time since last valid GPS fix
 * @return Time in milliseconds, or 0xFFFFFFFF if never had fix
 */
uint32_t GPS_GetTimeSinceLastFix(void);

/**
 * @brief Enable/disable GPS module (power management)
 * @param enable True to enable, false to disable
 * @return STATUS_OK on success
 */
StatusCode_t GPS_SetPowerState(bool enable);

/**
 * @brief Get GPS module status string
 * @param buffer Buffer to store status string
 * @param buffer_size Size of buffer
 * @return Number of characters written
 */
uint32_t GPS_GetStatusString(char *buffer, uint32_t buffer_size);

#endif /* GPS_DRIVER_H */
