/**
 * @file power_manager.c
 * @brief Power management implementation
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#include "power_manager.h"
#include "can_driver.h"
#include "adc_driver.h"
#include "gps_driver.h"

/******************************************************************************
 * Private Variables
 ******************************************************************************/

static PowerMode_t current_mode = POWER_MODE_ACTIVE;
static PowerMode_t previous_mode = POWER_MODE_ACTIVE;
static bool power_initialized = false;

static uint32_t idle_timeout_ms = IDLE_TIMEOUT_MS;
static uint32_t last_activity_time = 0;
static uint32_t mode_entry_time = 0;

// Power mode time tracking
static uint32_t time_active_ms = 0;
static uint32_t time_idle_ms = 0;
static uint32_t time_sleep_ms = 0;

// Energy tracking
static float cumulative_energy_mah = 0.0f;
static uint32_t last_energy_update_time = 0;

// Wake callbacks
static WakeCallback_t wake_callbacks[5] = {NULL};

// Peripheral state configurations for each mode
static PeripheralState_t peripheral_configs[3] = {
    // POWER_MODE_ACTIVE: All on
    {.can_enabled = true, .gps_enabled = true, .cellular_enabled = true, 
     .lorawan_enabled = true, .flash_enabled = true},
    
    // POWER_MODE_IDLE: Minimal active
    {.can_enabled = true, .gps_enabled = false, .cellular_enabled = false,
     .lorawan_enabled = false, .flash_enabled = false},
    
    // POWER_MODE_DEEP_SLEEP: All off except RTC
    {.can_enabled = false, .gps_enabled = false, .cellular_enabled = false,
     .lorawan_enabled = false, .flash_enabled = false}
};

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static void Power_ApplyPeripheralStates(const PeripheralState_t *states);
static void Power_UpdateEnergyConsumption(void);
static void Power_ConfigureWakeSources(PowerMode_t mode);

/******************************************************************************
 * Public Functions
 ******************************************************************************/

StatusCode_t Power_Init(void) {
    if (power_initialized) {
        return STATUS_OK;
    }

    current_mode = POWER_MODE_ACTIVE;
    last_activity_time = 0; // TODO: Get current system time
    mode_entry_time = last_activity_time;
    last_energy_update_time = last_activity_time;

    // Reset statistics
    time_active_ms = 0;
    time_idle_ms = 0;
    time_sleep_ms = 0;
    cumulative_energy_mah = 0.0f;

    power_initialized = true;

    DEBUG_PRINT("Power: Initialized in ACTIVE mode\n");
    return STATUS_OK;
}

StatusCode_t Power_SetMode(PowerMode_t mode) {
    if (!power_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (mode == current_mode) {
        return STATUS_OK;
    }

    // Update time statistics for previous mode
    uint32_t current_time = 0; // TODO: Get current system time
    uint32_t time_in_mode = current_time - mode_entry_time;

    switch (current_mode) {
        case POWER_MODE_ACTIVE:
            time_active_ms += time_in_mode;
            break;
        case POWER_MODE_IDLE:
            time_idle_ms += time_in_mode;
            break;
        case POWER_MODE_DEEP_SLEEP:
            time_sleep_ms += time_in_mode;
            break;
    }

    // Update energy consumption
    Power_UpdateEnergyConsumption();

    // Apply new peripheral states
    Power_ApplyPeripheralStates(&peripheral_configs[mode]);

    // Configure wake sources for new mode
    Power_ConfigureWakeSources(mode);

    // Transition to new mode
    previous_mode = current_mode;
    current_mode = mode;
    mode_entry_time = current_time;

    DEBUG_PRINT("Power: Mode changed from %d to %d\n", previous_mode, current_mode);

    // TODO: Hardware-specific power mode configuration
    switch (mode) {
        case POWER_MODE_ACTIVE:
            // Configure system clock to 80 MHz
            // Enable all required peripheral clocks
            break;

        case POWER_MODE_IDLE:
            // Enter CPU sleep mode (WFI instruction)
            // Peripherals remain clocked
            break;

        case POWER_MODE_DEEP_SLEEP:
            // Configure standby mode
            // Disable most clocks
            // Configure RTC wake-up
            // Execute WFI instruction
            break;
    }

    return STATUS_OK;
}

PowerMode_t Power_GetMode(void) {
    return current_mode;
}

StatusCode_t Power_RegisterWakeSource(WakeSource_t source, WakeCallback_t callback) {
    if (!power_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (source >= 5 || callback == NULL) {
        return STATUS_INVALID_PARAM;
    }

    wake_callbacks[source] = callback;

    DEBUG_PRINT("Power: Registered wake source %d\n", source);
    return STATUS_OK;
}

StatusCode_t Power_SetIdleTimeout(uint32_t timeout_ms) {
    idle_timeout_ms = timeout_ms;
    DEBUG_PRINT("Power: Idle timeout set to %lu ms\n", timeout_ms);
    return STATUS_OK;
}

float Power_GetCurrentConsumption(void) {
    switch (current_mode) {
        case POWER_MODE_ACTIVE:
            return CURRENT_ACTIVE_MA;
        case POWER_MODE_IDLE:
            return CURRENT_IDLE_MA;
        case POWER_MODE_DEEP_SLEEP:
            return CURRENT_DEEP_SLEEP_UA / 1000.0f; // Convert µA to mA
        default:
            return 0.0f;
    }
}

float Power_GetEnergyConsumption(void) {
    Power_UpdateEnergyConsumption();
    return cumulative_energy_mah;
}

void Power_ResetActivityTimer(void) {
    last_activity_time = 0; // TODO: Get current system time
}

uint32_t Power_GetTimeInMode(void) {
    uint32_t current_time = 0; // TODO: Get current system time
    return current_time - mode_entry_time;
}

StatusCode_t Power_GetStatistics(uint32_t *active_time_ms, uint32_t *idle_time_ms, uint32_t *sleep_time_ms) {
    if (active_time_ms == NULL || idle_time_ms == NULL || sleep_time_ms == NULL) {
        return STATUS_INVALID_PARAM;
    }

    // Include time in current mode
    uint32_t current_time_in_mode = Power_GetTimeInMode();

    *active_time_ms = time_active_ms;
    *idle_time_ms = time_idle_ms;
    *sleep_time_ms = time_sleep_ms;

    switch (current_mode) {
        case POWER_MODE_ACTIVE:
            *active_time_ms += current_time_in_mode;
            break;
        case POWER_MODE_IDLE:
            *idle_time_ms += current_time_in_mode;
            break;
        case POWER_MODE_DEEP_SLEEP:
            *sleep_time_ms += current_time_in_mode;
            break;
    }

    return STATUS_OK;
}

StatusCode_t Power_ConfigurePeripherals(PowerMode_t mode, const PeripheralState_t *states) {
    if (mode >= 3 || states == NULL) {
        return STATUS_INVALID_PARAM;
    }

    peripheral_configs[mode] = *states;
    DEBUG_PRINT("Power: Configured peripherals for mode %d\n", mode);

    return STATUS_OK;
}

StatusCode_t Power_SetPeripheralState(Component_t component, bool enable) {
    if (!power_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    DEBUG_PRINT("Power: %s component %d\n", enable ? "Enabling" : "Disabling", component);

    switch (component) {
        case COMPONENT_CAN:
            return CAN_SetPowerState(enable);
        case COMPONENT_GPS:
            return GPS_SetPowerState(enable);
        case COMPONENT_ADC:
            return ADC_SetPowerState(enable);
        // Add other components as needed
        default:
            return STATUS_INVALID_PARAM;
    }
}

StatusCode_t Power_EnterDeepSleep(void) {
    DEBUG_PRINT("Power: Forcing deep sleep\n");
    return Power_SetMode(POWER_MODE_DEEP_SLEEP);
}

/******************************************************************************
 * Private Functions
 ******************************************************************************/

static void Power_ApplyPeripheralStates(const PeripheralState_t *states) {
    if (states == NULL) {
        return;
    }

    Power_SetPeripheralState(COMPONENT_CAN, states->can_enabled);
    Power_SetPeripheralState(COMPONENT_GPS, states->gps_enabled);
    // Apply other peripheral states
}

static void Power_UpdateEnergyConsumption(void) {
    uint32_t current_time = 0; // TODO: Get current system time
    uint32_t elapsed_ms = current_time - last_energy_update_time;

    if (elapsed_ms == 0) {
        return;
    }

    float current_ma = Power_GetCurrentConsumption();
    float energy_mah = (current_ma * elapsed_ms) / 3600000.0f; // Convert ms to hours

    cumulative_energy_mah += energy_mah;
    last_energy_update_time = current_time;
}

static void Power_ConfigureWakeSources(PowerMode_t mode) {
    // TODO: Hardware-specific wake source configuration

    switch (mode) {
        case POWER_MODE_IDLE:
            // Enable: RTC alarm, CAN RX, ADC threshold
            break;

        case POWER_MODE_DEEP_SLEEP:
            // Enable: RTC alarm only
            break;

        default:
            break;
    }
}
