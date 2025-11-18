/**
 * @file can_driver.h
 * @brief CAN bus driver for vehicle speed data acquisition
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "telemetry_types.h"
#include "config.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief CAN RX callback function type
 * @param msg Pointer to received CAN message
 */
typedef void (*CANRxCallback_t)(const CANMessage_t *msg);

/******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize CAN peripheral
 * @param bitrate CAN bus bitrate in bps (e.g., 500000 for 500 kbps)
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t CAN_Init(uint32_t bitrate);

/**
 * @brief De-initialize CAN peripheral
 * @return STATUS_OK on success
 */
StatusCode_t CAN_Deinit(void);

/**
 * @brief Register callback for specific CAN message ID
 * @param id CAN message identifier to filter
 * @param callback Function to call when message is received
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t CAN_RegisterRxCallback(uint32_t id, CANRxCallback_t callback);

/**
 * @brief Transmit CAN message
 * @param msg Pointer to CAN message to transmit
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t CAN_Transmit(const CANMessage_t *msg);

/**
 * @brief Extract vehicle speed from standard CAN speed message
 * @param msg Pointer to CAN message (ID 0x200)
 * @return Vehicle speed in km/h, or -1.0 on error
 */
float CAN_ExtractSpeed(const CANMessage_t *msg);

/**
 * @brief Check if CAN bus is healthy
 * @return True if bus is operational, false otherwise
 */
bool CAN_IsHealthy(void);

/**
 * @brief Get last CAN message timestamp
 * @return Timestamp of last received message (milliseconds)
 */
uint32_t CAN_GetLastMessageTime(void);

/**
 * @brief Get CAN error statistics
 * @param tx_errors Pointer to store TX error count
 * @param rx_errors Pointer to store RX error count
 * @return STATUS_OK on success
 */
StatusCode_t CAN_GetErrorStats(uint32_t *tx_errors, uint32_t *rx_errors);

/**
 * @brief Enable/disable CAN peripheral (power management)
 * @param enable True to enable, false to disable
 * @return STATUS_OK on success
 */
StatusCode_t CAN_SetPowerState(bool enable);

#endif /* CAN_DRIVER_H */
