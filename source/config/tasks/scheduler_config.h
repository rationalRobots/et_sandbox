/**
 * @file scheduler_config.h
 * @brief Scheduler configuration definitions
 *
 * @copyright Copyright (c) 2025
 *
 * @author C Bird
 * @date 2025-11-07
 */

/******************************************************************************/
/* Scheduler Configuration Definitions                                        */
/******************************************************************************/

// Enable/disable all scheduler metrics collection

// Enable basic metrics only (last_exec_us, last_exec_delay_us, last_latency_us) without min/max tracking

// Enable event timestamp tracking for accurate event latency calculation (independent of other metrics)
// This provides accurate event timestamps for last_first_event_timestamp_us even when full metrics are disabled

// Enable detailed event latency tracking and per-event metrics (requires SCHEDULER_METRICS_ENABLED)
// This enables per-event latency arrays and detailed tracking in addition to event timestamps
