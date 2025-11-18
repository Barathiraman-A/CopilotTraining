/**
 * @file gps_driver.c
 * @brief GPS module driver implementation with NMEA parsing
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#include "gps_driver.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/******************************************************************************
 * Private Constants
 ******************************************************************************/

#define NMEA_BUFFER_SIZE    128
#define NMEA_MAX_TOKENS     20

/******************************************************************************
 * Private Variables
 ******************************************************************************/

static GPSData_t current_gps_data;
static bool gps_initialized = false;
static bool gps_has_fix = false;
static uint32_t last_fix_timestamp = 0;
static GPSDataCallback_t data_callback = NULL;

static char nmea_buffer[NMEA_BUFFER_SIZE];
static uint16_t nmea_index = 0;

/******************************************************************************
 * Private Function Prototypes
 ******************************************************************************/

static StatusCode_t GPS_ParseGPGGA(const char *sentence, GPSData_t *gps_data);
static StatusCode_t GPS_ParseGPRMC(const char *sentence, GPSData_t *gps_data);
static bool GPS_ValidateChecksum(const char *sentence);
static float GPS_ConvertCoordinate(const char *coord_str, const char *direction);
static void GPS_UARTCallback(uint8_t byte);

/******************************************************************************
 * Public Functions
 ******************************************************************************/

StatusCode_t GPS_Init(uint32_t baudrate) {
    if (gps_initialized) {
        return STATUS_OK;
    }

    // TODO: Hardware-specific UART initialization for STM32L476
    // 1. Enable UART2 and GPIO clocks
    // 2. Configure GPIO pins (TX: PA2, RX: PA3)
    // 3. Configure UART: baudrate, 8N1, RX interrupt enabled
    // 4. Enable UART peripheral

    // Initialize GPS data structure
    memset(&current_gps_data, 0, sizeof(GPSData_t));
    current_gps_data.valid = false;

    // Clear NMEA buffer
    memset(nmea_buffer, 0, sizeof(nmea_buffer));
    nmea_index = 0;

    gps_initialized = true;
    gps_has_fix = false;
    last_fix_timestamp = 0;

    DEBUG_PRINT("GPS: Initialized at %lu baud\n", baudrate);
    return STATUS_OK;
}

StatusCode_t GPS_Deinit(void) {
    if (!gps_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    // TODO: Hardware-specific de-initialization
    // 1. Disable UART interrupts
    // 2. Disable UART peripheral
    // 3. Disable clocks

    gps_initialized = false;
    data_callback = NULL;

    DEBUG_PRINT("GPS: De-initialized\n");
    return STATUS_OK;
}

StatusCode_t GPS_ParseNMEA(const char *sentence, GPSData_t *gps_data) {
    if (sentence == NULL || gps_data == NULL) {
        return STATUS_INVALID_PARAM;
    }

    // Validate checksum
    if (!GPS_ValidateChecksum(sentence)) {
        DEBUG_PRINT("GPS: Invalid checksum\n");
        return STATUS_ERROR;
    }

    // Determine sentence type and parse accordingly
    if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
        return GPS_ParseGPGGA(sentence, gps_data);
    } else if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
        return GPS_ParseGPRMC(sentence, gps_data);
    }

    // Unsupported sentence type
    return STATUS_ERROR;
}

StatusCode_t GPS_GetPosition(GPSData_t *gps_data) {
    if (!gps_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    if (gps_data == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (!current_gps_data.valid) {
        return STATUS_NO_DATA;
    }

    memcpy(gps_data, &current_gps_data, sizeof(GPSData_t));
    return STATUS_OK;
}

bool GPS_HasValidFix(void) {
    return gps_has_fix && current_gps_data.valid;
}

StatusCode_t GPS_RegisterCallback(GPSDataCallback_t callback) {
    if (!gps_initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    data_callback = callback;
    DEBUG_PRINT("GPS: Registered data callback\n");
    return STATUS_OK;
}

bool GPS_IsHealthy(void) {
    if (!gps_initialized) {
        return false;
    }

    // Check if we've received data recently
    uint32_t current_time = 0; // TODO: Get current system time
    if (last_fix_timestamp == 0) {
        return false; // Never received fix
    }

    uint32_t elapsed = current_time - last_fix_timestamp;
    if (elapsed > GPS_TIMEOUT_MS) {
        DEBUG_PRINT("GPS: Unhealthy - no fix for %lu ms\n", elapsed);
        return false;
    }

    // Check satellite count
    if (current_gps_data.satellites < GPS_MIN_SATELLITES) {
        DEBUG_PRINT("GPS: Unhealthy - only %d satellites\n", current_gps_data.satellites);
        return false;
    }

    return true;
}

uint32_t GPS_GetTimeSinceLastFix(void) {
    if (last_fix_timestamp == 0) {
        return 0xFFFFFFFF;
    }

    uint32_t current_time = 0; // TODO: Get current system time
    return current_time - last_fix_timestamp;
}

StatusCode_t GPS_SetPowerState(bool enable) {
    if (enable) {
        if (!gps_initialized) {
            return GPS_Init(GPS_UART_BAUDRATE);
        }
        // TODO: Send wake-up command to GPS module
        DEBUG_PRINT("GPS: Power enabled\n");
    } else {
        // TODO: Send sleep command to GPS module
        DEBUG_PRINT("GPS: Power disabled (sleep mode)\n");
    }

    return STATUS_OK;
}

uint32_t GPS_GetStatusString(char *buffer, uint32_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return 0;
    }

    if (!gps_initialized) {
        return snprintf(buffer, buffer_size, "GPS: Not initialized");
    }

    if (!gps_has_fix) {
        return snprintf(buffer, buffer_size, "GPS: No fix (Sats: %d)", 
                        current_gps_data.satellites);
    }

    return snprintf(buffer, buffer_size, 
                    "GPS: Fix OK | Lat: %.6f | Lon: %.6f | Sats: %d | Alt: %.1fm",
                    current_gps_data.latitude, current_gps_data.longitude,
                    current_gps_data.satellites, current_gps_data.altitude);
}

/******************************************************************************
 * Private Functions
 ******************************************************************************/

static StatusCode_t GPS_ParseGPGGA(const char *sentence, GPSData_t *gps_data) {
    // $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
    // Format: $GPGGA,time,lat,N/S,lon,E/W,quality,sats,hdop,alt,M,geoid,M,,*checksum

    char *tokens[NMEA_MAX_TOKENS];
    char sentence_copy[NMEA_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, NMEA_BUFFER_SIZE - 1);

    // Tokenize sentence
    uint8_t token_count = 0;
    char *token = strtok(sentence_copy, ",");
    while (token != NULL && token_count < NMEA_MAX_TOKENS) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }

    if (token_count < 10) {
        return STATUS_ERROR;
    }

    // Parse fix quality (token[6])
    gps_data->fix_quality = atoi(tokens[6]);
    if (gps_data->fix_quality == 0) {
        gps_data->valid = false;
        gps_has_fix = false;
        return STATUS_OK;
    }

    // Parse latitude (tokens[2], [3])
    gps_data->latitude = GPS_ConvertCoordinate(tokens[2], tokens[3]);

    // Parse longitude (tokens[4], [5])
    gps_data->longitude = GPS_ConvertCoordinate(tokens[4], tokens[5]);

    // Parse number of satellites (token[7])
    gps_data->satellites = atoi(tokens[7]);

    // Parse altitude (token[9])
    gps_data->altitude = atof(tokens[9]);

    // Parse HDOP (token[8])
    gps_data->hdop = (uint16_t)(atof(tokens[8]) * 100);

    gps_data->valid = true;
    gps_has_fix = true;
    last_fix_timestamp = 0; // TODO: Get current system time

    // Update global GPS data
    memcpy(&current_gps_data, gps_data, sizeof(GPSData_t));

    // Invoke callback if registered
    if (data_callback != NULL) {
        data_callback(gps_data);
    }

    return STATUS_OK;
}

static StatusCode_t GPS_ParseGPRMC(const char *sentence, GPSData_t *gps_data) {
    // $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
    // Format: $GPRMC,time,status,lat,N/S,lon,E/W,speed,track,date,mag_var,E/W*checksum

    char *tokens[NMEA_MAX_TOKENS];
    char sentence_copy[NMEA_BUFFER_SIZE];
    strncpy(sentence_copy, sentence, NMEA_BUFFER_SIZE - 1);

    // Tokenize sentence
    uint8_t token_count = 0;
    char *token = strtok(sentence_copy, ",");
    while (token != NULL && token_count < NMEA_MAX_TOKENS) {
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }

    if (token_count < 10) {
        return STATUS_ERROR;
    }

    // Parse status (token[2]): 'A' = valid, 'V' = invalid
    if (tokens[2][0] != 'A') {
        gps_data->valid = false;
        gps_has_fix = false;
        return STATUS_OK;
    }

    // Parse latitude (tokens[3], [4])
    gps_data->latitude = GPS_ConvertCoordinate(tokens[3], tokens[4]);

    // Parse longitude (tokens[5], [6])
    gps_data->longitude = GPS_ConvertCoordinate(tokens[5], tokens[6]);

    // Parse timestamp (token[1]): HHMMSS
    gps_data->timestamp = atoi(tokens[1]);

    gps_data->valid = true;
    gps_has_fix = true;
    last_fix_timestamp = 0; // TODO: Get current system time

    // Update global GPS data
    memcpy(&current_gps_data, gps_data, sizeof(GPSData_t));

    return STATUS_OK;
}

static bool GPS_ValidateChecksum(const char *sentence) {
    if (sentence == NULL || sentence[0] != '$') {
        return false;
    }

    // Find checksum delimiter
    const char *checksum_ptr = strchr(sentence, '*');
    if (checksum_ptr == NULL) {
        return false;
    }

    // Calculate checksum (XOR of all characters between $ and *)
    uint8_t calculated_checksum = 0;
    for (const char *p = sentence + 1; p < checksum_ptr; p++) {
        calculated_checksum ^= *p;
    }

    // Parse provided checksum (2 hex digits after *)
    uint8_t provided_checksum = 0;
    sscanf(checksum_ptr + 1, "%2hhx", &provided_checksum);

    return (calculated_checksum == provided_checksum);
}

static float GPS_ConvertCoordinate(const char *coord_str, const char *direction) {
    // NMEA format: DDMM.MMMM or DDDMM.MMMM
    if (coord_str == NULL || direction == NULL) {
        return 0.0f;
    }

    float coord = atof(coord_str);

    // Extract degrees and minutes
    int degrees = (int)(coord / 100);
    float minutes = coord - (degrees * 100);

    // Convert to decimal degrees
    float decimal_degrees = degrees + (minutes / 60.0f);

    // Apply direction (S and W are negative)
    if (direction[0] == 'S' || direction[0] == 'W') {
        decimal_degrees = -decimal_degrees;
    }

    return decimal_degrees;
}

static void GPS_UARTCallback(uint8_t byte) {
    // UART RX interrupt callback - accumulate NMEA sentence

    if (byte == '$') {
        // Start of new sentence
        nmea_index = 0;
        nmea_buffer[nmea_index++] = byte;
    } else if (byte == '\n') {
        // End of sentence
        nmea_buffer[nmea_index] = '\0';

        // Parse complete sentence
        GPSData_t gps_data;
        GPS_ParseNMEA(nmea_buffer, &gps_data);

        // Reset buffer
        nmea_index = 0;
    } else if (nmea_index < NMEA_BUFFER_SIZE - 1) {
        // Accumulate character
        nmea_buffer[nmea_index++] = byte;
    } else {
        // Buffer overflow - reset
        nmea_index = 0;
    }
}
