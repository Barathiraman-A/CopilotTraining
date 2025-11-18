/**
 * @file can_driver.c
 * @brief CAN bus driver implementation for STM32L476
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#include "can_driver.h"
#include <string.h>

/******************************************************************************
 * Private Variables
 ******************************************************************************/

static CANRxCallback_t rx_callbacks[16] = {NULL}; ///< RX callback array
static uint32_t filter_ids[16] = {0};             ///< Registered filter IDs
static uint8_t callback_count = 0;
static uint32_t last_rx_timestamp = 0;
static uint32_t tx_error_count = 0;
static uint32_t rx_error_count = 0;
static bool can_initialized = false;

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static void CAN_ConfigureFilters(void);
static void CAN_IRQHandler(void);

/******************************************************************************
 * Public Functions
 ******************************************************************************/

StatusCode_t CAN_Init(uint32_t bitrate) {
    if (can_initialized) {
        return STATUS_OK;
    }

    // TODO: Hardware-specific initialization for STM32L476 bxCAN
    // 1. Enable CAN peripheral clock
    // 2. Configure GPIO pins (CAN_TX, CAN_RX)
    // 3. Set bit timing registers based on bitrate
    // 4. Configure filter banks
    // 5. Enable FIFO0 interrupt
    // 6. Enter normal mode

    // Reset error counters
    tx_error_count = 0;
    rx_error_count = 0;
    last_rx_timestamp = 0;

    // Configure default filters
    CAN_ConfigureFilters();

    can_initialized = true;

    DEBUG_PRINT("CAN: Initialized at %lu bps\n", bitrate);
    return STATUS_OK;
}

StatusCode_t CAN_Deinit(void) {
    if (!can_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    // TODO: Hardware-specific de-initialization
    // 1. Disable CAN interrupts
    // 2. Enter initialization mode
    // 3. Disable CAN peripheral clock
    // 4. Reset GPIO pins

    can_initialized = false;
    callback_count = 0;
    memset(rx_callbacks, 0, sizeof(rx_callbacks));
    memset(filter_ids, 0, sizeof(filter_ids));

    DEBUG_PRINT("CAN: De-initialized\n");
    return STATUS_OK;
}

StatusCode_t CAN_RegisterRxCallback(uint32_t id, CANRxCallback_t callback) {
    if (!can_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (callback == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (callback_count >= 16) {
        return STATUS_BUFFER_FULL;
    }

    // Store callback and filter ID
    filter_ids[callback_count] = id;
    rx_callbacks[callback_count] = callback;
    callback_count++;

    // Reconfigure hardware filters
    CAN_ConfigureFilters();

    DEBUG_PRINT("CAN: Registered callback for ID 0x%lX\n", id);
    return STATUS_OK;
}

StatusCode_t CAN_Transmit(const CANMessage_t *msg) {
    if (!can_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (msg == NULL || msg->dlc > 8) {
        return STATUS_INVALID_PARAM;
    }

    // TODO: Hardware-specific transmission
    // 1. Check if TX mailbox is available
    // 2. Load message ID, DLC, and data
    // 3. Request transmission
    // 4. Wait for completion or timeout

    DEBUG_PRINT("CAN: Transmit ID 0x%lX, DLC %d\n", msg->id, msg->dlc);
    return STATUS_OK;
}

float CAN_ExtractSpeed(const CANMessage_t *msg) {
    if (msg == NULL || msg->id != CAN_SPEED_MSG_ID || msg->dlc < 2) {
        return -1.0f;
    }

    // Extract speed: bytes 0-1, big-endian, value in (km/h * 100)
    uint16_t speed_raw = ((uint16_t)msg->data[0] << 8) | msg->data[1];
    float speed_kmh = speed_raw / 100.0f;

    return speed_kmh;
}

bool CAN_IsHealthy(void) {
    if (!can_initialized) {
        return false;
    }

    // Check if we've received messages recently
    uint32_t current_time = 0; // TODO: Get current system time in ms
    uint32_t elapsed = current_time - last_rx_timestamp;

    if (elapsed > CAN_TIMEOUT_MS) {
        DEBUG_PRINT("CAN: Timeout - no messages for %lu ms\n", elapsed);
        return false;
    }

    // Check error thresholds
    if (tx_error_count > 100 || rx_error_count > 100) {
        DEBUG_PRINT("CAN: Error count too high (TX: %lu, RX: %lu)\n", 
                    tx_error_count, rx_error_count);
        return false;
    }

    return true;
}

uint32_t CAN_GetLastMessageTime(void) {
    return last_rx_timestamp;
}

StatusCode_t CAN_GetErrorStats(uint32_t *tx_errors, uint32_t *rx_errors) {
    if (tx_errors == NULL || rx_errors == NULL) {
        return STATUS_INVALID_PARAM;
    }

    *tx_errors = tx_error_count;
    *rx_errors = rx_error_count;

    return STATUS_OK;
}

StatusCode_t CAN_SetPowerState(bool enable) {
    if (!can_initialized && enable) {
        return STATUS_NOT_INITIALIZED;
    }

    if (enable) {
        // TODO: Wake CAN peripheral from sleep
        DEBUG_PRINT("CAN: Power enabled\n");
    } else {
        // TODO: Put CAN peripheral to sleep mode
        DEBUG_PRINT("CAN: Power disabled (sleep mode)\n");
    }

    return STATUS_OK;
}

/******************************************************************************
 * Private Functions
 ******************************************************************************/

static void CAN_ConfigureFilters(void) {
    // TODO: Configure hardware filter banks
    // For each registered callback, add filter for corresponding ID
    // Use mask mode to allow exact ID matching

    DEBUG_PRINT("CAN: Configured %d filters\n", callback_count);
}

/**
 * @brief CAN interrupt handler (called by HAL)
 */
static void CAN_IRQHandler(void) {
    CANMessage_t msg;

    // TODO: Hardware-specific RX processing
    // 1. Check FIFO0 for pending messages
    // 2. Read message ID, DLC, and data
    // 3. Update timestamp
    // 4. Call registered callbacks

    last_rx_timestamp = 0; // TODO: Get current system time

    // Find and invoke matching callback
    for (uint8_t i = 0; i < callback_count; i++) {
        if (msg.id == filter_ids[i] && rx_callbacks[i] != NULL) {
            rx_callbacks[i](&msg);
            break;
        }
    }
}

// TODO: Register CAN_IRQHandler with NVIC for CAN RX interrupts
