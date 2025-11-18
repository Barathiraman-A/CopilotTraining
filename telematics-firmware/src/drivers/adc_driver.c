/**
 * @file adc_driver.c
 * @brief ADC driver implementation for battery voltage monitoring
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#include "adc_driver.h"
#include <string.h>

/******************************************************************************
 * Private Constants
 ******************************************************************************/

#define ADC_BUFFER_SIZE         16      ///< DMA circular buffer size
#define ADC_MAX_VALUE           4095    ///< 12-bit maximum value

/******************************************************************************
 * Private Variables
 ******************************************************************************/

static uint16_t adc_buffer[ADC_BUFFER_SIZE];
static bool adc_initialized = false;
static bool adc_running = false;

// Calibration parameters (stored in flash in production)
static float adc_offset = 0.0f;
static float adc_scale = 1.0f;

// Threshold monitoring
static float voltage_threshold = 0.0f;
static ADCThresholdCallback_t threshold_callback = NULL;
static bool threshold_triggered = false;

// Statistics
static float voltage_min = 999.0f;
static float voltage_max = 0.0f;
static float voltage_sum = 0.0f;
static uint32_t sample_count = 0;

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static float ADC_RawToVoltage(uint16_t raw_value);
static void ADC_DMACallback(void);
static void ADC_CheckThreshold(float voltage);

/******************************************************************************
 * Public Functions
 ******************************************************************************/

StatusCode_t ADC_Init(void) {
    if (adc_initialized) {
        return STATUS_OK;
    }

    // TODO: Hardware-specific initialization for STM32L476 ADC
    // 1. Enable ADC and GPIO clocks
    // 2. Configure GPIO pin (PA0) as analog input
    // 3. Configure ADC: 12-bit resolution, continuous mode
    // 4. Configure DMA for circular buffer transfer
    // 5. Calibrate ADC offset

    // Reset statistics
    voltage_min = 999.0f;
    voltage_max = 0.0f;
    voltage_sum = 0.0f;
    sample_count = 0;

    // Clear buffer
    memset(adc_buffer, 0, sizeof(adc_buffer));

    adc_initialized = true;

    DEBUG_PRINT("ADC: Initialized (12-bit, DMA circular buffer)\n");
    return STATUS_OK;
}

StatusCode_t ADC_Deinit(void) {
    if (!adc_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    ADC_StopConversion();

    // TODO: Hardware-specific de-initialization
    // 1. Disable ADC and DMA
    // 2. Disable clocks
    // 3. Reset GPIO pin

    adc_initialized = false;

    DEBUG_PRINT("ADC: De-initialized\n");
    return STATUS_OK;
}

StatusCode_t ADC_StartConversion(void) {
    if (!adc_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (adc_running) {
        return STATUS_OK;
    }

    // TODO: Hardware-specific start
    // 1. Enable DMA requests
    // 2. Start ADC conversion

    adc_running = true;

    DEBUG_PRINT("ADC: Conversion started\n");
    return STATUS_OK;
}

StatusCode_t ADC_StopConversion(void) {
    if (!adc_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (!adc_running) {
        return STATUS_OK;
    }

    // TODO: Hardware-specific stop
    // 1. Stop ADC conversion
    // 2. Disable DMA requests

    adc_running = false;

    DEBUG_PRINT("ADC: Conversion stopped\n");
    return STATUS_OK;
}

float ADC_GetBatteryVoltage(void) {
    if (!adc_initialized || !adc_running) {
        return -1.0f;
    }

    // Calculate average of all samples in buffer for noise reduction
    uint32_t sum = 0;
    for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++) {
        sum += adc_buffer[i];
    }
    uint16_t avg_raw = sum / ADC_BUFFER_SIZE;

    float voltage = ADC_RawToVoltage(avg_raw);

    // Update statistics
    if (voltage < voltage_min) voltage_min = voltage;
    if (voltage > voltage_max) voltage_max = voltage;
    voltage_sum += voltage;
    sample_count++;

    // Check threshold
    ADC_CheckThreshold(voltage);

    return voltage;
}

uint16_t ADC_GetRawValue(void) {
    if (!adc_initialized || !adc_running) {
        return 0xFFFF;
    }

    // Return most recent sample
    return adc_buffer[0];
}

StatusCode_t ADC_RegisterThresholdCallback(float threshold, ADCThresholdCallback_t callback) {
    if (!adc_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (callback == NULL || threshold <= 0.0f) {
        return STATUS_INVALID_PARAM;
    }

    voltage_threshold = threshold;
    threshold_callback = callback;
    threshold_triggered = false;

    DEBUG_PRINT("ADC: Registered threshold callback at %.2f V\n", threshold);
    return STATUS_OK;
}

StatusCode_t ADC_Calibrate(float known_voltage) {
    if (!adc_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    uint16_t raw = ADC_GetRawValue();
    if (raw == 0xFFFF) {
        return STATUS_NO_DATA;
    }

    // Calculate scale factor
    float uncalibrated = ADC_RawToVoltage(raw);
    adc_scale = known_voltage / uncalibrated;

    DEBUG_PRINT("ADC: Calibrated - scale factor: %.4f\n", adc_scale);
    // TODO: Store calibration to flash for persistence

    return STATUS_OK;
}

bool ADC_IsHealthy(void) {
    if (!adc_initialized || !adc_running) {
        return false;
    }

    // Check if ADC values are reasonable (not stuck)
    uint16_t first = adc_buffer[0];
    bool all_same = true;
    for (uint16_t i = 1; i < ADC_BUFFER_SIZE; i++) {
        if (adc_buffer[i] != first) {
            all_same = false;
            break;
        }
    }

    if (all_same) {
        DEBUG_PRINT("ADC: Unhealthy - values stuck at %u\n", first);
        return false;
    }

    return true;
}

StatusCode_t ADC_SetPowerState(bool enable) {
    if (enable) {
        if (!adc_initialized) {
            return ADC_Init();
        }
        return ADC_StartConversion();
    } else {
        return ADC_StopConversion();
    }
}

StatusCode_t ADC_GetStatistics(float *min_voltage, float *max_voltage, float *avg_voltage) {
    if (min_voltage == NULL || max_voltage == NULL || avg_voltage == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (sample_count == 0) {
        return STATUS_NO_DATA;
    }

    *min_voltage = voltage_min;
    *max_voltage = voltage_max;
    *avg_voltage = voltage_sum / sample_count;

    return STATUS_OK;
}

/******************************************************************************
 * Private Functions
 ******************************************************************************/

static float ADC_RawToVoltage(uint16_t raw_value) {
    // Convert raw ADC value to voltage
    // V = (raw / 4095) * VREF * voltage_divider
    float voltage = (raw_value / (float)ADC_MAX_VALUE) * 
                    (ADC_VREF_MV / 1000.0f) * 
                    ADC_VOLTAGE_DIVIDER;

    // Apply calibration
    voltage = (voltage + adc_offset) * adc_scale;

    return voltage;
}

static void ADC_DMACallback(void) {
    // Called by DMA interrupt when buffer is half-full or full
    // Can be used for real-time processing if needed
}

static void ADC_CheckThreshold(float voltage) {
    if (threshold_callback == NULL || voltage_threshold <= 0.0f) {
        return;
    }

    // Check if voltage dropped below threshold
    if (voltage < voltage_threshold && !threshold_triggered) {
        threshold_triggered = true;
        threshold_callback(voltage);
        DEBUG_PRINT("ADC: Threshold triggered - voltage: %.2f V\n", voltage);
    }

    // Reset trigger if voltage recovers
    if (voltage >= voltage_threshold + 0.5f) {
        threshold_triggered = false;
    }
}
