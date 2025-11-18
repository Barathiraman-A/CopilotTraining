/**
 * @file power_manager.h
 * @brief Power management system for low-power operation
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "telemetry_types.h"
#include "config.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Wake callback function type
 * @param source Wake source that triggered the wake-up
 */
typedef void (*WakeCallback_t)(WakeSource_t source);

/**
 * @brief Peripheral state configuration
 */
typedef struct {
    bool can_enabled;
    bool gps_enabled;
    bool cellular_enabled;
    bool lorawan_enabled;
    bool flash_enabled;
} PeripheralState_t;

/******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize power management system
 * @return STATUS_OK on success
 */
StatusCode_t Power_Init(void);

/**
 * @brief Set power mode
 * @param mode Target power mode
 * @return STATUS_OK on success
 */
StatusCode_t Power_SetMode(PowerMode_t mode);

/**
 * @brief Get current power mode
 * @return Current power mode
 */
PowerMode_t Power_GetMode(void);

/**
 * @brief Register wake source callback
 * @param source Wake source identifier
 * @param callback Function to call on wake from this source
 * @return STATUS_OK on success
 */
StatusCode_t Power_RegisterWakeSource(WakeSource_t source, WakeCallback_t callback);

/**
 * @brief Configure idle timeout
 * @param timeout_ms Timeout in milliseconds
 * @return STATUS_OK on success
 */
StatusCode_t Power_SetIdleTimeout(uint32_t timeout_ms);

/**
 * @brief Get estimated current consumption
 * @return Current consumption in mA
 */
float Power_GetCurrentConsumption(void);

/**
 * @brief Get cumulative energy consumption
 * @return Energy consumption in mAh
 */
float Power_GetEnergyConsumption(void);

/**
 * @brief Reset activity timer (prevents idle timeout)
 */
void Power_ResetActivityTimer(void);

/**
 * @brief Get time in current power mode
 * @return Time in milliseconds
 */
uint32_t Power_GetTimeInMode(void);

/**
 * @brief Get power mode statistics
 * @param active_time_ms Time spent in ACTIVE mode
 * @param idle_time_ms Time spent in IDLE mode
 * @param sleep_time_ms Time spent in DEEP_SLEEP mode
 * @return STATUS_OK on success
 */
StatusCode_t Power_GetStatistics(uint32_t *active_time_ms, uint32_t *idle_time_ms, uint32_t *sleep_time_ms);

/**
 * @brief Configure peripheral power states for each mode
 * @param mode Power mode
 * @param states Peripheral state configuration
 * @return STATUS_OK on success
 */
StatusCode_t Power_ConfigurePeripherals(PowerMode_t mode, const PeripheralState_t *states);

/**
 * @brief Enable/disable peripheral
 * @param component Component to control
 * @param enable True to enable, false to disable
 * @return STATUS_OK on success
 */
StatusCode_t Power_SetPeripheralState(Component_t component, bool enable);

/**
 * @brief Force immediate transition to DEEP_SLEEP
 * @return STATUS_OK on success
 */
StatusCode_t Power_EnterDeepSleep(void);

#endif /* POWER_MANAGER_H */
