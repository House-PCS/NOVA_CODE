/**
 ******************************************************************************
 * @file    shell_netif.h
 * @author  NovaStar R&D Team
 * @version V1.0.0
 * @date    2017.09.28
 * @brief
 ******************************************************************************
 * @attention
 *
 * History<br>
 * version     date        auther      description<br>
 * V1.0.0      2017.11.13  Huyuxin     Initial Version<br>
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MID_SHELL_H
#define __MID_SHELL_H

/* Inculdes ------------------------------------------------------------------*/

#include "bsp.h"
#include "string.h"

/* Exported typedef -----------------------------------------------------------*/






typedef uint32_t (*consloe_fun_msh)(uint32_t argc, uint8_t **argv);
typedef void (*consloe_fun)(void);

typedef struct _console_cmd
{
    const char     *name;
    const char     *des;
    void (*fun)();
} _console_cmd_t, *_pconsole_cmd_t;

typedef struct _console_cmd_msh
{
    const char     *name;
    const char     *des;
    consloe_fun_msh fun;
} _console_cmd_msh_t, *_pconsole_cmd_msh_t;
/* Exported define ------------------------------------------------------------*/
/*                                  console-start                              */
#define SECTION(x) __attribute__((used)) __attribute__((section(x)))
//This macro pragma console fun to "FConTab" fields
#define SHELL_FUNCTION_EXPORT_CMD(name, cmd, desc)      \
                const char _console_##cmd##_name[] = #cmd;            \
                const char _console_##cmd##_desc[] = #desc;           \
                const struct _console_cmd _console_##cmd SECTION("FConTab")= \
                {                           \
                    _console_##cmd##_name,    \
                    _console_##cmd##_desc,    \
                    (consloe_fun)name     \
                };

#define SHELL_FUNCTION_EXPORT_CMD_MSH(name, cmd, desc)      \
                const char _console_msh_##cmd##_name[] = #cmd;            \
                const char _console_msh_##cmd##_desc[] = #desc;           \
                const struct _console_cmd_msh _console_msh_##cmd SECTION("FConTabMsh")= \
                {                           \
                    _console_msh_##cmd##_name,    \
                    _console_msh_##cmd##_desc,    \
                    (consloe_fun_msh)name     \
                };
//This macro exports a system function to console shell.

#define FINSH_CMD_EXPORT(name, desc)      SHELL_FUNCTION_EXPORT_CMD(name, name, desc)
#define MSH_CMD_EXPORT(name, desc)         SHELL_FUNCTION_EXPORT_CMD_MSH(name, name, desc)
/*                                  console-end                               */

/* Exported constants --------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
void console_init(void);
void console_recive_handler(const uint8_t * p_buffer, uint32_t n_length);

#endif /*__MID_SHELL_H */

