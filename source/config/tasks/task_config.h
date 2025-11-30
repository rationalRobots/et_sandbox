/**
 * @file task_config.h
 * @brief Task configuration definitions
 *
 * @copyright Copyright (c) 2025
 *
 * @author C Bird
 * @date 2025-09-18
 */


/******************************************************************************/
/* Public Type Definitions                                                    */
/******************************************************************************/

typedef enum
{
    TASK_ID_DEBUG = 0U,
    TASK_ID_STATUS,
    TASK_ID_MOTOR,
    TASK_ID_ETHERCAT,
    TASK_ID_COMMS,
    TASK_ID_POWER,

    TASK_ID_COUNT

} tTask_id;

typedef enum
{
    TASK_EVT_NONE = 0U,

    TASK_EVT_DEBUG_RX,
    TASK_EVT_COMMS_RX,
    TASK_EVT_DC_SYNC,
    TASK_EVT_MOTOR_FAULT,
    TASK_EVT_SAFETY_RELAY_CHANGE,   /* the safety relay has changed state */
    TASK_EVT_POWER_GOOD,            /* TODO: one or more of the power rails has reported power good */
    TASK_EVT_POWER_STATE_CHANGE,    /* TODO: one or more of the power rails has changed state */

    TASK_EVENT_COUNT

} tTask_eventFlagIds;

/******************************************************************************/
/* Public Function Declarations                                               */
/******************************************************************************/

/**
 * @brief Returns the board specific task configuration table.
 *
 * @return const tTask*
 */
const tTaskConfig *task_config_getTaskTable(void);
