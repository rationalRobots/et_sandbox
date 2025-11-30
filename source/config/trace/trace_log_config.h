/**
 * @file trace_log_config.h
 * @brief App configuration for the trace_log library
 *
 * @copyright Copyright (c) 2025
 *
 * @author C Bird
 * @date 2025-10-08
 *
 */
#ifndef TRACE_LOG_CONFIG_H
#define TRACE_LOG_CONFIG_H

#include "trace_log_types.h"

#include <stdbool.h>
#include <stdint.h>

// Enable output function instead of printf

#define TRACE_LOG_MAX_MESSAGE_SIZE 256
#define TRACE_LOG_UART_QUEUE_SIZE 32
#define TRACE_LOG_UART_MESSAGE_SIZE TRACE_LOG_MAX_MESSAGE_SIZE

#define TRACE_LOG_SNPRINTF snprintf
#define TRACE_LOG_VSNPRINTF vsnprintf
#define TRACE_LOG_PRINTF printf

#define TRACE_LOG_MODULE_LEVEL_ENABLED(module_id, level) \
    ((level) <= trace_log_config_moduleLevels[module_id])

// UART DMA output configuration

/******************************************************************************/
/* Public Type Definitions                                                    */
/******************************************************************************/

/**
 * @brief Message queue entry for UART DMA output
 */
typedef struct
{
    char     message[TRACE_LOG_UART_MESSAGE_SIZE];
    uint16_t length;
    bool     in_use;
} tTraceLogUartMessage;

/**
 * @brief UART DMA output queue
 */
typedef struct
{
    tTraceLogUartMessage messages[TRACE_LOG_UART_QUEUE_SIZE];
    uint8_t              head;
    uint8_t              tail;
    uint8_t              count;
    bool                 dma_busy;
} tTraceLogUartQueue;

/**
 * @brief Trace log module identifiers
 */
typedef enum
{
    TID_KERNEL = 0U,
    TID_SM,
    TID_MAIN,
    TID_DEBUG,
    TID_POWER,
    TID_COMMS,
    TID_MOTOR,
    TID_ETHERCAT,
    TID_TMC9660,
    TID_TMC9660_BL,
    TID_TMC9660_SPI,
    TID_SPI,

    TID_NUM_MODULES

} tTraceModule;

/******************************************************************************/
/* Public Function Declarations                                               */
/******************************************************************************/

/**
 * @brief Initialize the trace log UART output system
 */
void trace_log_init(void);

/**
 * @brief UART DMA output function for trace logging
 *
 * @param message The formatted message string to output
 * @return tTraceLogResult Result of the output operation
 */
tTraceLogResult trace_log_uart_output(const char *message);

/**
 * @brief Get debug information about the trace log system
 *
 * @param callback_count_out Pointer to store callback count
 * @param queue_count_out Pointer to store current queue count
 * @param dma_busy_out Pointer to store DMA busy status
 */
void trace_log_get_debug_info(uint32_t *callback_count_out, uint8_t *queue_count_out, bool *dma_busy_out);

void trace_log_tx_complete_callback(void);

#endif // TRACE_LOG_CONFIG_H