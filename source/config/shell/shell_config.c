/**
 * @file shell_config.c
 * @brief Shell configuration and global context implementation
 *
 * @copyright Copyright (c) 2025
 *
 * @author C Bird
 * @date 2025-11-14
 */



/******************************************************************************/
/* Private Global Variables                                                   */
/******************************************************************************/

static shell_config_t shell_cfg;
static volatile int shell_initialized = 0;

/******************************************************************************/
/* Public Global Variables                                                   */
/******************************************************************************/

shell_uart_context_t g_shell_ctx;

/******************************************************************************/
/* Private Function Declarations                                              */
/******************************************************************************/

/**
 * @brief Shell command to show firmware version
 */
static int app_cmd_show_version(int argc, char **argv, shell_io_t *io);

/******************************************************************************/
/* Public Function Definitions                                                */
/******************************************************************************/

int shell_config_init(void)
{
    int rc;
    shell_t *shell_instance;

    TRACE_LOG(TID_DEBUG, TL_INFO, "Initializing shell configuration...\r\n");

    /* Configure shell parameters */
    shell_cfg.io = NULL;  /* Will be set by shell_uart_init */
    shell_cfg.prompt = NULL;
    shell_cfg.max_line_len = 128u;
    shell_cfg.max_tokens = 12u;
    shell_cfg.max_nodes = 48u;

    /* Initialize shell UART integration */
    rc = shell_uart_init(&g_shell_ctx, &huart2, &shell_cfg);
    if (rc != 0) {
        TRACE_LOG(TID_DEBUG, TL_ERROR, "Shell UART init failed: %d\r\n", rc);
        return rc;
    }

    /* Get shell instance from context */
    shell_instance = shell_uart_get_shell(&g_shell_ctx);
    if (shell_instance == NULL) {
        TRACE_LOG(TID_DEBUG, TL_ERROR, "Failed to get shell instance\r\n");
        return -1;
    }

    /* Configure shell */
    rc = shell_set_prompt_prefix(shell_instance, "evc");
    if (rc != 0) {
        TRACE_LOG(TID_DEBUG, TL_WARN, "Failed to set shell prompt prefix\r\n");
    }

    /* Set password and optionally start locked */
    (void)shell_set_lock_password(shell_instance, "opensesame");
    /* (void)shell_lock(shell_instance); */

    /* Built-ins (help, ls, cd, reset, lock, unlock) */
    (void)shell_register_builtins(shell_instance);

    /* Root utilities */
    (void)shell_register_cmd(shell_instance, "/", "version", "Show FW version", app_cmd_show_version);

    /* Directory tree and module commands */
    (void)shell_register_dir(shell_instance, "/", "sys", "System");
    (void)shell_register_dir(shell_instance, "/sys", "info", "System info");

    /* Start UART reception */
    rc = shell_uart_start_rx(&g_shell_ctx);
    if (rc != 0) {
        TRACE_LOG(TID_DEBUG, TL_ERROR, "Failed to start shell UART RX: %d\r\n", rc);
        return rc;
    }

    /* Mark as initialized */
    shell_initialized = 1;

    TRACE_LOG(TID_DEBUG, TL_INFO, "Shell configuration initialized successfully\r\n");
    return 0;
}

int shell_config_is_initialized(void)
{
    return shell_initialized;
}

shell_t *shell_config_get_shell(void)
{
    if (!shell_initialized) {
        return NULL;
    }
    return shell_uart_get_shell(&g_shell_ctx);
}

int shell_config_process_rx(void)
{
    if (!shell_initialized) {
        return 0;
    }

    return shell_uart_process_rx(&g_shell_ctx);
}

/******************************************************************************/
/* Private Function Definitions                                               */
/******************************************************************************/

static int app_cmd_show_version(int argc, char **argv, shell_io_t *io)
{
    (void) argc;
    (void) argv;

    if (io && io->write) {
        char buffer[128];
        int len = snprintf(buffer, sizeof(buffer), "%s v%d.%d.%d (%s) (%s)\r\n",
                          (BUILD_IS_DIRTY ? "DIRTY!!!" : ""),
                          VERSION_MAJOR_NUM, VERSION_MINOR_NUM, VERSION_PATCH_NUM,
                          VERSION_STR, BRANCH_STR);
        if (len > 0) {
            io->write(io, buffer, (size_t)len);
        }
    }

    TRACE_LOG(TID_DEBUG, TL_STARTUP, "%s v%d.%d.%d (%s) (%s)\r\n",
              (BUILD_IS_DIRTY ? "DIRTY!!!" : ""),
              VERSION_MAJOR_NUM, VERSION_MINOR_NUM, VERSION_PATCH_NUM,
              VERSION_STR, BRANCH_STR);

    return 0;
}
