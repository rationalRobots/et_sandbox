/**
 * @file shell_config.h
 * @brief Shell configuration and global context
 *
 * @copyright Copyright (c) 2025
 *
 * @author C Bird
 * @date 2025-11-14
 */



extern "C" {

/******************************************************************************/
/* Public Global Variables                                                   */
/******************************************************************************/

/**
 * @brief Global shell UART context - accessible from all modules
 */
extern shell_uart_context_t g_shell_ctx;

/******************************************************************************/
/* Public Function Declarations                                              */
/******************************************************************************/

/**
 * @brief Initialize shell configuration and start shell UART
 *
 * This function should be called early in main() after UART initialization
 * but before any tasks that might need shell access.
 *
 * @return 0 on success, negative on error
 */
int shell_config_init(void);

/**
 * @brief Check if shell is initialized and ready
 *
 * @return 1 if shell is initialized, 0 if not
 */
int shell_config_is_initialized(void);

/**
 * @brief Get shell instance from global context
 *
 * @return Pointer to shell instance or NULL if not initialized
 */
shell_t *shell_config_get_shell(void);

/**
 * @brief Process shell RX data - call periodically from main loop or task
 *
 * This function is safe to call even if shell is not initialized.
 *
 * @return Number of characters processed, or 0 if shell not initialized
 */
int shell_config_process_rx(void);

}
