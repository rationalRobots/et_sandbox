/**
 * @file task_config.c
 * @brief Task configuration definitions
 *
 * @copyright Copyright (c) 2025
 *
 * @author C Bird
 * @date 2025-09-18
 */


/******************************************************************************/
/* Private Global Variables                                                   */
/******************************************************************************/

// clang-format off
static const tTaskConfig m_task_config[ TASK_ID_COUNT] =
{
    [TASK_ID_COMMS] =
    {
        .name        = "Comms",
        .priority    = TASK_PRIORITY_HIGHEST - 1,
        .timeout_us  = 1000000U,
        .deadline_us = 1500000U,
        .event_flags = { .bits = EVENT_FLAG(TASK_EVT_COMMS_RX) }
    },
    [TASK_ID_MOTOR] =
    {
        .name        = "Motor",
        .priority    = TASK_PRIORITY_HIGHEST,
        .timeout_us  = 1000U, // 1ms periodic check (TMC wdg 250ms?) // TODO [ ] lower task period later
        .deadline_us = 1500U,
        .event_flags = {
            .bits =
            (
                // TASK_EVT_NONE
                EVENT_FLAG(TASK_EVT_MOTOR_FAULT) |
                EVENT_FLAG(TASK_EVT_SAFETY_RELAY_CHANGE) |
                EVENT_FLAG(TASK_EVT_POWER_STATE_CHANGE)
            )
        }
    },
    [TASK_ID_ETHERCAT] =
    {
        .name        = "EtherCAT",
        .priority    = TASK_PRIORITY_HIGHEST - 1,
        .timeout_us  = 1000000U,
        .deadline_us = 10000000U,
        .event_flags = { .bits = EVENT_FLAG(TASK_EVT_DC_SYNC) }
    },
    [TASK_ID_POWER] =
    {
        .name        = "Power",
        .priority    = TASK_PRIORITY_LOWEST + 2,
        .timeout_us   = 10000U,  // 10ms periodic check
        .deadline_us = 200000U, // 20ms deadline
        .event_flags = { .bits =
            (
                EVENT_FLAG(TASK_EVT_SAFETY_RELAY_CHANGE) |
                EVENT_FLAG(TASK_EVT_POWER_GOOD)
            )
        }
    },
    [TASK_ID_STATUS] =
    {
        .name        = "Status",
        .priority    = TASK_PRIORITY_LOWEST + 1,
        .timeout_us  = 10000000U,
        .deadline_us = 20000000U,
        .event_flags = { .bits = (EVENT_FLAG(TASK_EVT_MOTOR_FAULT) | EVENT_FLAG(TASK_EVT_COMMS_RX)) }
    },
    [TASK_ID_DEBUG] =
    {
        .name        = "Debug",
        .priority    = TASK_PRIORITY_LOWEST,
        .timeout_us  = 100000U,
        .deadline_us = 0U,
        .event_flags = { .bits = TASK_EVT_NONE }
    },
};
// clang-format on

/******************************************************************************/
/* Public Function Definitions                                               */
/******************************************************************************/

const tTaskConfig *task_config_getTaskTable(void)
{
    return m_task_config;
}
