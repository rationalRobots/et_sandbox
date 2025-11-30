/**
 * @file trace_log_config.c
 * @brief App configuration for the trace log module
 *
 * @copyright Copyright (c) 2025  plc.
 *
 * @author C Bird
 * @date 2025-10-08
 *
 */
#include "trace_log_config.h"

#include "trace_log.h"
#include "port.h"

#include "stm32h533xx.h"
#include "stm32h5xx_hal.h"
#include "usart.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


/******************************************************************************/
/* Private Global Variables                                                   */
/******************************************************************************/

/**
 * @brief UART DMA output queue
 */
static tTraceLogUartQueue uart_queue          = { 0 };
static uint32_t           trace_dropped_count = 0U;

/**
 * @brief Debug counter for callback invocations
 */
static volatile uint32_t callback_count = 0;

/******************************************************************************/
/* Public Global Variables                                                    */
/******************************************************************************/

// clang-format off
/**
 * @brief Trace log module levels for filtering
 */
tTraceLogLevel trace_log_config_moduleLevels[TID_NUM_MODULES] =
{
    [TID_KERNEL]            = TL_WARN,
    [TID_SM]                = TL_WARN,
    [TID_MAIN]              = TL_WARN,
    [TID_DEBUG]             = TL_WARN,
    [TID_POWER]             = TL_INFO,
    [TID_COMMS]             = TL_ERROR,
    [TID_MOTOR]             = TL_INFO,
    [TID_ETHERCAT]          = TL_DEBUG,
    [TID_TMC9660]           = TL_INFO,
    [TID_TMC9660_BL]        = TL_DEBUG,
    [TID_TMC9660_SPI]       = TL_INFO,
    [TID_SPI]               = TL_INFO
};
// clang-format on

/**
 * @brief Output function for trace logging
 */
tTraceLogOutputFunc trace_log_output_func = trace_log_uart_output;

/******************************************************************************/
/* Private Function Declarations                                              */
/******************************************************************************/

static tTraceLogResult uart_queue_enqueue(const char *message);
static tTraceLogResult uart_queue_dequeue(void);
static tTraceLogResult uart_start_transmission(void);
static bool            uart_queue_is_full(void);
static bool            uart_queue_is_empty(void);

/******************************************************************************/
/* Public Function Definitions                                                */
/******************************************************************************/

/**
 * @brief Get the timestamp for the trace logger
 *
 * @return uint32_t The current timestamp in milliseconds
 */
uint32_t _trace_log_get_timestamp_ms(void)
{
    return port_get_time_ms();
}

/**
 * @brief Get the timestamp for the trace logger with microsecond precision
 *
 * @return uint32_t The current timestamp in microseconds
 */
uint32_t _trace_log_get_timestamp_us(void)
{
    return port_get_time_us();
}

/**
 * @brief Initialize the trace log UART output system
 */
void trace_log_init(void)
{
    // Initialize the queue
    memset(&uart_queue, 0, sizeof(uart_queue));
    uart_queue.head     = 0;
    uart_queue.tail     = 0;
    uart_queue.count    = 0;
    uart_queue.dma_busy = false;
}

/**
 * @brief UART DMA output function for trace logging
 *
 * @param message The formatted message string to output
 * @return tTraceLogResult Result of the output operation
 */
tTraceLogResult trace_log_uart_output(const char *message)
{
    if(message == NULL)
    {

        return TL_RESULT_INVALID_PARAM;
    }

    // Check if queue is full
    if(uart_queue_is_full())
    {
        trace_dropped_count++;

        return TL_RESULT_BUFFER_FULL;
    }

    // Add message to queue
    tTraceLogResult result = uart_queue_enqueue(message);
    if(result != TL_RESULT_OK)
    {
        return result;
    }

    // If DMA is not busy, start transmission immediately
    if(!uart_queue.dma_busy)
    {
        result = uart_start_transmission();
    }

    return result;
}

/**
 * @brief Callback function called when UART transmission is complete
 *
 * @return * void
 */
void trace_log_tx_complete_callback(void)
{
    // Increment callback counter for debugging
    callback_count++;

    // Mark current transmission as complete
    uart_queue.dma_busy = false;

    // Dequeue the completed message
    if(uart_queue_dequeue() == TL_RESULT_OK)
    {
        // Start next transmission if queue is not empty
        if(!uart_queue_is_empty())
        {
            uart_start_transmission();
        }
    }
}

/******************************************************************************/
/* Private Function Definitions                                               */
/******************************************************************************/

/**
 * @brief Add a message to the UART output queue
 *
 * @param message Message to add to queue
 * @return tTraceLogResult Result of operation
 */
static tTraceLogResult uart_queue_enqueue(const char *message)
{
    if(uart_queue_is_full())
    {
        // trace_dropped_count++; // this should never be reached as checked in layer above

        return TL_RESULT_BUFFER_FULL;
    }

    if(trace_dropped_count > 0U)
    {
        // Log dropped message count
        char drop_msg[64];
        snprintf(drop_msg, sizeof(drop_msg), "Dropped messages: %lu\r\n", trace_dropped_count);

        trace_dropped_count = 0U;

        uart_queue_enqueue(drop_msg); // TODO !! remove recursion .. replace with direct queue insert??
        // TODO ^ this will fill the buffer and prevent the log below !
    }

    tTraceLogUartMessage *msg = &uart_queue.messages[uart_queue.tail];

    // Copy message and calculate length
    strncpy(msg->message, message, TRACE_LOG_UART_MESSAGE_SIZE - 1);
    msg->message[TRACE_LOG_UART_MESSAGE_SIZE - 1] = '\0';
    msg->length                                   = strlen(msg->message);
    msg->in_use                                   = true;

    // Update queue pointers
    uart_queue.tail = (uart_queue.tail + 1) % TRACE_LOG_UART_QUEUE_SIZE;
    uart_queue.count++;

    return TL_RESULT_OK;
}

/**
 * @brief Remove a message from the UART output queue
 *
 * @return tTraceLogResult Result of operation
 */
static tTraceLogResult uart_queue_dequeue(void)
{
    if(uart_queue_is_empty())
    {
        return TL_RESULT_ERROR;
    }

    // Mark message as not in use
    uart_queue.messages[uart_queue.head].in_use = false;

    // Update queue pointers
    uart_queue.head = (uart_queue.head + 1) % TRACE_LOG_UART_QUEUE_SIZE;
    uart_queue.count--;

    return TL_RESULT_OK;
}

/**
 * @brief Start UART DMA transmission of the next message in queue
 *
 * @return tTraceLogResult Result of operation
 */
static tTraceLogResult uart_start_transmission(void)
{
    if(uart_queue_is_empty())
    {
        return TL_RESULT_ERROR;
    }

    if(uart_queue.dma_busy)
    {
        return TL_RESULT_ERROR;
    }

    // Check UART state before starting transmission
    if(huart2.gState != HAL_UART_STATE_READY)
    {
        return TL_RESULT_ERROR;
    }

    // TODO log dropped  message count

    tTraceLogUartMessage *msg = &uart_queue.messages[uart_queue.head];

    // Validate message
    if(msg->length == 0 || msg->length > TRACE_LOG_UART_MESSAGE_SIZE)
    {
        // Invalid message, skip it
        uart_queue_dequeue();
        return TL_RESULT_ERROR;
    }

    // Start DMA transmission
    uart_queue.dma_busy          = true; // TODO temp workaround
    HAL_StatusTypeDef hal_result = HAL_UART_Transmit_DMA(&huart2, (uint8_t *)msg->message, msg->length);

    if(hal_result != HAL_OK)
    {
        uart_queue.dma_busy = false;
        return TL_RESULT_OK;
    }
    // else
    // {
    //     // DMA start failed, reset state
    //     uart_queue.dma_busy = false;
    return TL_RESULT_ERROR;
    // }
}

/**
 * @brief Check if the UART queue is full
 *
 * @return true if queue is full, false otherwise
 */
static bool uart_queue_is_full(void)
{
    return (uart_queue.count >= TRACE_LOG_UART_QUEUE_SIZE);
}

/**
 * @brief Check if the UART queue is empty
 *
 * @return true if queue is empty, false otherwise
 */
static bool uart_queue_is_empty(void)
{
    return (uart_queue.count == 0);
}

int __io_putchar(int ch)
{
    while(!(USART1->ISR & USART_ISR_TXE_Msk))
    {
        __NOP();
    }

    USART1->TDR = (uint8_t)ch;

    return ch;
}
