/**
 * @file circular_buffer.h
 * @brief Lock-free circular buffer for telemetry data
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "telemetry_types.h"
#include "config.h"

/******************************************************************************
 * Type Definitions
 ******************************************************************************/

/**
 * @brief Circular buffer structure for telemetry records
 */
typedef struct {
    TelemetryRecord_t records[TELEMETRY_BUFFER_SIZE];
    volatile uint32_t write_index;
    volatile uint32_t read_index;
    volatile uint32_t count;
    uint32_t overflow_count;
    bool initialized;
} CircularBuffer_t;

/******************************************************************************
 * Public API
 ******************************************************************************/

/**
 * @brief Initialize circular buffer
 * @param buffer Pointer to circular buffer structure
 * @return STATUS_OK on success
 */
StatusCode_t CircularBuffer_Init(CircularBuffer_t *buffer);

/**
 * @brief Push telemetry record into buffer (non-blocking)
 * @param buffer Pointer to circular buffer
 * @param record Pointer to telemetry record to push
 * @return STATUS_OK on success, STATUS_BUFFER_FULL if buffer is full
 */
StatusCode_t CircularBuffer_Push(CircularBuffer_t *buffer, const TelemetryRecord_t *record);

/**
 * @brief Pop telemetry record from buffer (non-blocking)
 * @param buffer Pointer to circular buffer
 * @param record Pointer to store popped record
 * @return STATUS_OK on success, STATUS_NO_DATA if buffer is empty
 */
StatusCode_t CircularBuffer_Pop(CircularBuffer_t *buffer, TelemetryRecord_t *record);

/**
 * @brief Peek at record without removing it
 * @param buffer Pointer to circular buffer
 * @param record Pointer to store peeked record
 * @param offset Offset from read position (0 = next record)
 * @return STATUS_OK on success, STATUS_NO_DATA if offset out of range
 */
StatusCode_t CircularBuffer_Peek(const CircularBuffer_t *buffer, TelemetryRecord_t *record, uint32_t offset);

/**
 * @brief Get number of records currently in buffer
 * @param buffer Pointer to circular buffer
 * @return Number of records
 */
uint32_t CircularBuffer_GetCount(const CircularBuffer_t *buffer);

/**
 * @brief Check if buffer is full
 * @param buffer Pointer to circular buffer
 * @return True if full, false otherwise
 */
bool CircularBuffer_IsFull(const CircularBuffer_t *buffer);

/**
 * @brief Check if buffer is empty
 * @param buffer Pointer to circular buffer
 * @return True if empty, false otherwise
 */
bool CircularBuffer_IsEmpty(const CircularBuffer_t *buffer);

/**
 * @brief Get buffer utilization percentage
 * @param buffer Pointer to circular buffer
 * @return Utilization (0-100%)
 */
uint8_t CircularBuffer_GetUtilization(const CircularBuffer_t *buffer);

/**
 * @brief Clear all data from buffer
 * @param buffer Pointer to circular buffer
 * @return STATUS_OK on success
 */
StatusCode_t CircularBuffer_Clear(CircularBuffer_t *buffer);

/**
 * @brief Get overflow count (number of dropped records)
 * @param buffer Pointer to circular buffer
 * @return Overflow count
 */
uint32_t CircularBuffer_GetOverflowCount(const CircularBuffer_t *buffer);

/**
 * @brief Pop multiple records in batch
 * @param buffer Pointer to circular buffer
 * @param records Array to store popped records
 * @param max_count Maximum number of records to pop
 * @return Number of records actually popped
 */
uint32_t CircularBuffer_PopBatch(CircularBuffer_t *buffer, TelemetryRecord_t *records, uint32_t max_count);

#endif /* CIRCULAR_BUFFER_H */
