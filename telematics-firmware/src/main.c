/**
 * @file main.c
 * @brief Main application entry point for vehicle telematics firmware
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#include <stdio.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "config.h"
#include "telemetry_types.h"
#include "can_driver.h"
#include "adc_driver.h"
#include "gps_driver.h"
#include "circular_buffer.h"
#include "power_manager.h"

/******************************************************************************
 * Global Variables
 ******************************************************************************/

static CircularBuffer_t telemetry_buffer;
static TaskHandle_t sensor_task_handle = NULL;
static TaskHandle_t data_process_task_handle = NULL;
static TaskHandle_t comm_tx_task_handle = NULL;
static TaskHandle_t power_mgr_task_handle = NULL;
static TaskHandle_t health_mon_task_handle = NULL;

/******************************************************************************
 * Task Prototypes
 ******************************************************************************/

static void SensorAcquisitionTask(void *pvParameters);
static void DataProcessingTask(void *pvParameters);
static void CommunicationTxTask(void *pvParameters);
static void PowerManagementTask(void *pvParameters);
static void HealthMonitorTask(void *pvParameters);

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static void System_Init(void);
static void CreateTasks(void);
static uint16_t Calculate_CRC16(const uint8_t *data, uint16_t length);

/******************************************************************************
 * Main Function
 ******************************************************************************/

int main(void) {
    // Initialize system
    System_Init();

    printf("\n");
    printf("==========================================\n");
    printf("  Vehicle Telematics Unit Firmware\n");
    printf("  Version: %d.%d.%d\n", FIRMWARE_VERSION_MAJOR, 
           FIRMWARE_VERSION_MINOR, FIRMWARE_VERSION_PATCH);
    printf("  Build Date: %s\n", FIRMWARE_BUILD_DATE);
    printf("==========================================\n\n");

    // Create FreeRTOS tasks
    CreateTasks();

    // Start FreeRTOS scheduler
    printf("Starting FreeRTOS scheduler...\n");
    vTaskStartScheduler();

    // Should never reach here
    while (1) {
        __asm__ __volatile__("nop");
    }

    return 0;
}

/******************************************************************************
 * System Initialization
 ******************************************************************************/

static void System_Init(void) {
    // TODO: Hardware-specific initialization
    // 1. Configure system clocks (80 MHz)
    // 2. Initialize GPIO
    // 3. Configure NVIC priorities
    // 4. Initialize systick timer

    // Initialize circular buffer
    StatusCode_t status = CircularBuffer_Init(&telemetry_buffer);
    if (status != STATUS_OK) {
        printf("ERROR: Failed to initialize circular buffer\n");
    }

    // Initialize drivers
    status = CAN_Init(CAN_BITRATE);
    if (status != STATUS_OK) {
        printf("ERROR: Failed to initialize CAN driver\n");
    }

    status = ADC_Init();
    if (status != STATUS_OK) {
        printf("ERROR: Failed to initialize ADC driver\n");
    }

    status = GPS_Init(GPS_UART_BAUDRATE);
    if (status != STATUS_OK) {
        printf("ERROR: Failed to initialize GPS driver\n");
    }

    // Initialize power manager
    status = Power_Init();
    if (status != STATUS_OK) {
        printf("ERROR: Failed to initialize power manager\n");
    }

    // Start ADC conversion
    ADC_StartConversion();

    printf("System initialization complete\n");
}

/******************************************************************************
 * Task Creation
 ******************************************************************************/

static void CreateTasks(void) {
    BaseType_t result;

    // Sensor Acquisition Task (HIGH priority)
    result = xTaskCreate(
        SensorAcquisitionTask,
        "SensorAcq",
        STACK_SIZE_SENSOR_ACQ / sizeof(StackType_t),
        NULL,
        PRIORITY_SENSOR_ACQ,
        &sensor_task_handle
    );
    if (result != pdPASS) {
        printf("ERROR: Failed to create SensorAcquisitionTask\n");
    }

    // Data Processing Task (MEDIUM priority)
    result = xTaskCreate(
        DataProcessingTask,
        "DataProc",
        STACK_SIZE_DATA_PROCESS / sizeof(StackType_t),
        NULL,
        PRIORITY_DATA_PROCESS,
        &data_process_task_handle
    );
    if (result != pdPASS) {
        printf("ERROR: Failed to create DataProcessingTask\n");
    }

    // Communication TX Task (MEDIUM priority)
    result = xTaskCreate(
        CommunicationTxTask,
        "CommTx",
        STACK_SIZE_COMM_TX / sizeof(StackType_t),
        NULL,
        PRIORITY_COMM_TX,
        &comm_tx_task_handle
    );
    if (result != pdPASS) {
        printf("ERROR: Failed to create CommunicationTxTask\n");
    }

    // Power Management Task (LOW priority)
    result = xTaskCreate(
        PowerManagementTask,
        "PowerMgr",
        STACK_SIZE_POWER_MGR / sizeof(StackType_t),
        NULL,
        PRIORITY_POWER_MGR,
        &power_mgr_task_handle
    );
    if (result != pdPASS) {
        printf("ERROR: Failed to create PowerManagementTask\n");
    }

    // Health Monitor Task (LOW priority)
    result = xTaskCreate(
        HealthMonitorTask,
        "HealthMon",
        STACK_SIZE_HEALTH_MON / sizeof(StackType_t),
        NULL,
        PRIORITY_HEALTH_MON,
        &health_mon_task_handle
    );
    if (result != pdPASS) {
        printf("ERROR: Failed to create HealthMonitorTask\n");
    }

    printf("All tasks created successfully\n");
}

/******************************************************************************
 * Sensor Acquisition Task
 ******************************************************************************/

static void SensorAcquisitionTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t sampling_period = pdMS_TO_TICKS(1000); // 1 Hz sampling

    printf("SensorAcquisitionTask started\n");

    while (1) {
        // Create telemetry record
        TelemetryRecord_t record;
        memset(&record, 0, sizeof(TelemetryRecord_t));

        // Get current timestamp
        record.timestamp = 0; // TODO: Get RTC timestamp (Unix epoch)

        // Read vehicle speed from CAN
        // (In real implementation, speed would come from CAN callback)
        record.speed = 0.0f;
        if (CAN_IsHealthy()) {
            record.flags |= FLAG_CAN_VALID;
        }

        // Read battery voltage from ADC
        record.battery_voltage = ADC_GetBatteryVoltage();
        if (record.battery_voltage > 0.0f) {
            record.flags |= FLAG_ADC_VALID;
            
            // Check low battery threshold
            if (record.battery_voltage < BATTERY_LOW_THRESHOLD_V) {
                record.flags |= FLAG_LOW_BATTERY;
            }
        }

        // Read GPS position
        GPSData_t gps_data;
        if (GPS_GetPosition(&gps_data) == STATUS_OK) {
            record.latitude = gps_data.latitude;
            record.longitude = gps_data.longitude;
            record.altitude = gps_data.altitude;
            record.gps_satellites = gps_data.satellites;
            record.gps_fix_quality = gps_data.fix_quality;
            record.flags |= FLAG_GPS_VALID;
        }

        // Calculate CRC16 checksum
        record.crc16 = Calculate_CRC16((uint8_t *)&record, sizeof(TelemetryRecord_t) - 2);

        // Push record into circular buffer
        StatusCode_t status = CircularBuffer_Push(&telemetry_buffer, &record);
        if (status == STATUS_BUFFER_FULL) {
            printf("WARNING: Telemetry buffer overflow\n");
        }

        // Debug output
        uint8_t utilization = CircularBuffer_GetUtilization(&telemetry_buffer);
        printf("Telemetry: Speed=%.1f km/h, Battery=%.2fV, GPS=(%.6f,%.6f), Sats=%d, Buffer=%d%%\n",
               record.speed, record.battery_voltage, record.latitude, record.longitude,
               record.gps_satellites, utilization);

        // Wait for next sampling period
        vTaskDelayUntil(&last_wake_time, sampling_period);
    }
}

/******************************************************************************
 * Data Processing Task
 ******************************************************************************/

static void DataProcessingTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t processing_period = pdMS_TO_TICKS(500); // 2 Hz

    printf("DataProcessingTask started\n");

    while (1) {
        // Check buffer utilization
        uint8_t utilization = CircularBuffer_GetUtilization(&telemetry_buffer);

        // If buffer > 50% full, trigger transmission
        if (utilization > 50) {
            printf("Buffer high (%d%%) - triggering transmission\n", utilization);
            // TODO: Set event flag for communication task
        }

        // TODO: Implement data compression if enabled
        // TODO: Implement data packaging for transmission

        vTaskDelayUntil(&last_wake_time, processing_period);
    }
}

/******************************************************************************
 * Communication TX Task
 ******************************************************************************/

static void CommunicationTxTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t tx_period = pdMS_TO_TICKS(CELLULAR_TX_INTERVAL_MS);

    printf("CommunicationTxTask started\n");

    while (1) {
        // Get batch of telemetry records
        TelemetryRecord_t records[32];
        uint32_t count = CircularBuffer_PopBatch(&telemetry_buffer, records, 32);

        if (count > 0) {
            printf("Transmitting %lu telemetry records\n", count);

            // TODO: Compress data if enabled
            // TODO: Encrypt data if enabled
            // TODO: Transmit via cellular or LoRaWAN
            // TODO: Implement retry logic on failure
        }

        vTaskDelayUntil(&last_wake_time, tx_period);
    }
}

/******************************************************************************
 * Power Management Task
 ******************************************************************************/

static void PowerManagementTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t check_period = pdMS_TO_TICKS(5000); // 5 seconds

    printf("PowerManagementTask started\n");

    while (1) {
        // Get current power statistics
        float current_ma = Power_GetCurrentConsumption();
        float energy_mah = Power_GetEnergyConsumption();

        printf("Power: Mode=%d, Current=%.1f mA, Energy=%.2f mAh\n",
               Power_GetMode(), current_ma, energy_mah);

        // TODO: Implement idle timeout logic
        // TODO: Check activity and transition to lower power modes

        vTaskDelayUntil(&last_wake_time, check_period);
    }
}

/******************************************************************************
 * Health Monitor Task
 ******************************************************************************/

static void HealthMonitorTask(void *pvParameters) {
    (void)pvParameters;

    TickType_t last_wake_time = xTaskGetTickCount();
    const TickType_t monitor_period = pdMS_TO_TICKS(HEALTH_CHECK_INTERVAL_MS);

    printf("HealthMonitorTask started\n");

    while (1) {
        // Check component health
        bool gps_healthy = GPS_IsHealthy();
        bool can_healthy = CAN_IsHealthy();
        bool adc_healthy = ADC_IsHealthy();

        if (!gps_healthy) {
            printf("WARNING: GPS unhealthy\n");
        }
        if (!can_healthy) {
            printf("WARNING: CAN unhealthy\n");
        }
        if (!adc_healthy) {
            printf("WARNING: ADC unhealthy\n");
        }

        // TODO: Check temperature
        // TODO: Check memory integrity
        // TODO: Log errors to flash
        // TODO: Trigger alerts if needed

        // Refresh watchdog
        // TODO: Watchdog_Refresh();

        vTaskDelayUntil(&last_wake_time, monitor_period);
    }
}

/******************************************************************************
 * Utility Functions
 ******************************************************************************/

static uint16_t Calculate_CRC16(const uint8_t *data, uint16_t length) {
    // CRC-16/CCITT-FALSE algorithm
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/******************************************************************************
 * FreeRTOS Hook Functions
 ******************************************************************************/

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    printf("CRITICAL: Stack overflow in task: %s\n", pcTaskName);
    while (1) {
        // Halt system
    }
}

void vApplicationMallocFailedHook(void) {
    printf("CRITICAL: Malloc failed\n");
    while (1) {
        // Halt system
    }
}
