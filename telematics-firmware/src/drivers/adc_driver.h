/**
 * @file adc_driver.h
 * @brief ADC driver for battery voltage monitoring
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include "telemetry_types.h"
#include "config.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief ADC threshold callback function type
 * @param voltage Current battery voltage in volts
 */
typedef void (*ADCThresholdCallback_t)(float voltage);

/******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize ADC peripheral with DMA
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t ADC_Init(void);

/**
 * @brief De-initialize ADC peripheral
 * @return STATUS_OK on success
 */
StatusCode_t ADC_Deinit(void);

/**
 * @brief Start continuous ADC conversion with DMA
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t ADC_StartConversion(void);

/**
 * @brief Stop ADC conversion
 * @return STATUS_OK on success
 */
StatusCode_t ADC_StopConversion(void);

/**
 * @brief Get latest battery voltage reading
 * @return Battery voltage in volts, or -1.0 on error
 */
float ADC_GetBatteryVoltage(void);

/**
 * @brief Get raw ADC value (12-bit)
 * @return Raw ADC value (0-4095), or 0xFFFF on error
 */
uint16_t ADC_GetRawValue(void);

/**
 * @brief Register callback for voltage threshold
 * @param threshold Voltage threshold in volts
 * @param callback Function to call when voltage drops below threshold
 * @return STATUS_OK on success, error code otherwise
 */
StatusCode_t ADC_RegisterThresholdCallback(float threshold, ADCThresholdCallback_t callback);

/**
 * @brief Perform ADC calibration
 * @param known_voltage Actual voltage measured by external meter
 * @return STATUS_OK on success
 */
StatusCode_t ADC_Calibrate(float known_voltage);

/**
 * @brief Check if ADC is healthy and producing valid data
 * @return True if healthy, false otherwise
 */
bool ADC_IsHealthy(void);

/**
 * @brief Enable/disable ADC peripheral (power management)
 * @param enable True to enable, false to disable
 * @return STATUS_OK on success
 */
StatusCode_t ADC_SetPowerState(bool enable);

/**
 * @brief Get ADC statistics
 * @param min_voltage Pointer to store minimum voltage
 * @param max_voltage Pointer to store maximum voltage
 * @param avg_voltage Pointer to store average voltage
 * @return STATUS_OK on success
 */
StatusCode_t ADC_GetStatistics(float *min_voltage, float *max_voltage, float *avg_voltage);

#endif /* ADC_DRIVER_H */
