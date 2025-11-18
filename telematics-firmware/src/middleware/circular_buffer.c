/**
 * @file circular_buffer.c
 * @brief Lock-free circular buffer implementation
 * @author Embedded Systems Team
 * @date 2025-11-18
 */

#include "circular_buffer.h"
#include <string.h>

/******************************************************************************
 * Private Macros
 ******************************************************************************/

#define ATOMIC_INCREMENT(var) __atomic_fetch_add(&(var), 1, __ATOMIC_SEQ_CST)
#define ATOMIC_LOAD(var) __atomic_load_n(&(var), __ATOMIC_SEQ_CST)
#define ATOMIC_STORE(var, val) __atomic_store_n(&(var), (val), __ATOMIC_SEQ_CST)

/******************************************************************************
 * Public Functions
 ******************************************************************************/

StatusCode_t CircularBuffer_Init(CircularBuffer_t *buffer) {
    if (buffer == NULL) {
        return STATUS_INVALID_PARAM;
    }

    // Clear buffer memory
    memset(buffer, 0, sizeof(CircularBuffer_t));

    buffer->write_index = 0;
    buffer->read_index = 0;
    buffer->count = 0;
    buffer->overflow_count = 0;
    buffer->initialized = true;

    DEBUG_PRINT("CircularBuffer: Initialized (capacity: %d records)\n", 
                TELEMETRY_BUFFER_SIZE);
    return STATUS_OK;
}

StatusCode_t CircularBuffer_Push(CircularBuffer_t *buffer, const TelemetryRecord_t *record) {
    if (buffer == NULL || record == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (!buffer->initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    // Check if buffer is full
    uint32_t current_count = ATOMIC_LOAD(buffer->count);
    if (current_count >= TELEMETRY_BUFFER_SIZE) {
        // Buffer full - overwrite oldest record (FIFO behavior)
        ATOMIC_INCREMENT(buffer->overflow_count);
        
        // Advance read index to discard oldest record
        uint32_t old_read = ATOMIC_LOAD(buffer->read_index);
        uint32_t new_read = (old_read + 1) % TELEMETRY_BUFFER_SIZE;
        ATOMIC_STORE(buffer->read_index, new_read);
        
        // Decrement count to make space
        __atomic_fetch_sub(&buffer->count, 1, __ATOMIC_SEQ_CST);
        
        DEBUG_PRINT("CircularBuffer: Overflow - oldest record discarded\n");
    }

    // Get write index and copy record
    uint32_t write_idx = ATOMIC_LOAD(buffer->write_index);
    memcpy(&buffer->records[write_idx], record, sizeof(TelemetryRecord_t));

    // Update write index (circular)
    uint32_t new_write_idx = (write_idx + 1) % TELEMETRY_BUFFER_SIZE;
    ATOMIC_STORE(buffer->write_index, new_write_idx);

    // Increment count
    ATOMIC_INCREMENT(buffer->count);

    return STATUS_OK;
}

StatusCode_t CircularBuffer_Pop(CircularBuffer_t *buffer, TelemetryRecord_t *record) {
    if (buffer == NULL || record == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (!buffer->initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    // Check if buffer is empty
    uint32_t current_count = ATOMIC_LOAD(buffer->count);
    if (current_count == 0) {
        return STATUS_NO_DATA;
    }

    // Get read index and copy record
    uint32_t read_idx = ATOMIC_LOAD(buffer->read_index);
    memcpy(record, &buffer->records[read_idx], sizeof(TelemetryRecord_t));

    // Update read index (circular)
    uint32_t new_read_idx = (read_idx + 1) % TELEMETRY_BUFFER_SIZE;
    ATOMIC_STORE(buffer->read_index, new_read_idx);

    // Decrement count
    __atomic_fetch_sub(&buffer->count, 1, __ATOMIC_SEQ_CST);

    return STATUS_OK;
}

StatusCode_t CircularBuffer_Peek(const CircularBuffer_t *buffer, TelemetryRecord_t *record, uint32_t offset) {
    if (buffer == NULL || record == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (!buffer->initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    // Check if offset is within available data
    uint32_t current_count = ATOMIC_LOAD(buffer->count);
    if (offset >= current_count) {
        return STATUS_NO_DATA;
    }

    // Calculate peek index
    uint32_t read_idx = ATOMIC_LOAD(buffer->read_index);
    uint32_t peek_idx = (read_idx + offset) % TELEMETRY_BUFFER_SIZE;

    // Copy record without modifying buffer state
    memcpy(record, &buffer->records[peek_idx], sizeof(TelemetryRecord_t));

    return STATUS_OK;
}

uint32_t CircularBuffer_GetCount(const CircularBuffer_t *buffer) {
    if (buffer == NULL || !buffer->initialized) {
        return 0;
    }

    return ATOMIC_LOAD(buffer->count);
}

bool CircularBuffer_IsFull(const CircularBuffer_t *buffer) {
    if (buffer == NULL || !buffer->initialized) {
        return false;
    }

    return (ATOMIC_LOAD(buffer->count) >= TELEMETRY_BUFFER_SIZE);
}

bool CircularBuffer_IsEmpty(const CircularBuffer_t *buffer) {
    if (buffer == NULL || !buffer->initialized) {
        return true;
    }

    return (ATOMIC_LOAD(buffer->count) == 0);
}

uint8_t CircularBuffer_GetUtilization(const CircularBuffer_t *buffer) {
    if (buffer == NULL || !buffer->initialized) {
        return 0;
    }

    uint32_t count = ATOMIC_LOAD(buffer->count);
    return (uint8_t)((count * 100) / TELEMETRY_BUFFER_SIZE);
}

StatusCode_t CircularBuffer_Clear(CircularBuffer_t *buffer) {
    if (buffer == NULL) {
        return STATUS_INVALID_PARAM;
    }

    if (!buffer->initialized) {
        return STATUS_NOT_INITIALIZED;
    }

    // Reset indices and count
    ATOMIC_STORE(buffer->write_index, 0);
    ATOMIC_STORE(buffer->read_index, 0);
    ATOMIC_STORE(buffer->count, 0);

    DEBUG_PRINT("CircularBuffer: Cleared\n");
    return STATUS_OK;
}

uint32_t CircularBuffer_GetOverflowCount(const CircularBuffer_t *buffer) {
    if (buffer == NULL || !buffer->initialized) {
        return 0;
    }

    return ATOMIC_LOAD(buffer->overflow_count);
}

uint32_t CircularBuffer_PopBatch(CircularBuffer_t *buffer, TelemetryRecord_t *records, uint32_t max_count) {
    if (buffer == NULL || records == NULL || max_count == 0) {
        return 0;
    }

    if (!buffer->initialized) {
        return 0;
    }

    uint32_t count = 0;
    for (uint32_t i = 0; i < max_count; i++) {
        if (CircularBuffer_Pop(buffer, &records[i]) != STATUS_OK) {
            break;
        }
        count++;
    }

    return count;
}
