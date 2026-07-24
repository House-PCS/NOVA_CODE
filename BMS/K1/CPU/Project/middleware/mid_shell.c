/**
 ******************************************************************************
 * @file    mid_shell.c
 * @author  NovaStar R&D Team
 * @version V1.0.0
 * @date    2017.09.28
 * @brief   控制台的控制实现
 ******************************************************************************
 * @attention
 *
 * History<br>
 * version     date        auther      description<br>
 * V1.0.0      2017.11.13  Huyuxin     Initial Version<br>
 * V1.1.0      2022.03.29  Huyuxin     支持msh命令方式<br>
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "mid_shell.h"

/* Gloabal variables ---------------------------------------------------------*/

/* External variables ---------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define LOG_TAG "SHELL"

#define _CMDFUN_NAME_LENGTH_MAX           (100)    /*交互指令中名字最长限度*/
#define _CMDFUN_CACHE_NUM_MAX             (15)      /*交互指令缓存最大个数*/
#define _CMDFUN_ARG_NUM_MAX               (50)     /*交互指令中参数最多限度*/
#define _CMDFUN_SPACE_X30                 ("                              ")

#define MSH_MODE (1)
#define FINSH_MODE (0)

static char *p_name = NULL;
static char MSH_NAME[] = "MSH>>";
static char FINSH_NAME[] = "FINSH>>";
/* Private typedef -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static _pconsole_cmd_t  _spConsoleTableBegin = NULL;            /*预编译“FConTab”段中起始的交互指令*/
static _pconsole_cmd_t  _spConsoleTableEnd   = NULL;            /*预编译“FConTab”段中结尾的交互指令*/
static _pconsole_cmd_msh_t  _spConsoleTableBeginMsh = NULL;            /*预编译“FConTab”段中起始的交互指令*/
static _pconsole_cmd_msh_t  _spConsoleTableEndMsh   = NULL;            /*预编译“FConTab”段中结尾的交互指令*/
static uint8_t s_shell_mode = MSH_MODE;
/* Private function prototypes -----------------------------------------------*/
static void ConsoleFunctioninit(const void* begin, const void* end, uint8_t mode);
static uint8_t console_get_fun_name(char* name, const uint8_t *pbuf, uint32_t len);
static int32_t console_msh_get_arg(uint8_t *argv[_CMDFUN_ARG_NUM_MAX], uint8_t *cmd, uint32_t length);
static void * ConsoleFunctionLookUp(const char* name, uint8_t mode);
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                  Private Function
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

static void _get_control_name(void)
{
    if(s_shell_mode == MSH_MODE)
    {
        p_name = MSH_NAME;
    }
    else if(s_shell_mode == FINSH_MODE)
    {
        p_name = FINSH_NAME;
    }
}

/******************************************************************************************************
  * @brief  加载预编译到指定字段交互命令的头尾
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
static void ConsoleFunctioninit(const void* begin, const void* end, uint8_t mode)
{
    if(mode != MSH_MODE)
    {
        _spConsoleTableBegin = (_pconsole_cmd_t) begin;
        _spConsoleTableEnd = (_pconsole_cmd_t) end;
    }
    else
    {
        _spConsoleTableBeginMsh = (_pconsole_cmd_msh_t) begin;
        _spConsoleTableEndMsh = (_pconsole_cmd_msh_t) end;
    }
}
/******************************************************************************************************
  * @brief  获取交互命令的名字
  * @param
  * @param
  * @retval 名字长度
  *****************************************************************************************************/
static uint8_t console_get_fun_name(char* name, const uint8_t *pbuf, uint32_t len)
{
    uint32_t i   = 0;
    uint8_t  ret = 0;
    char c_mode = '(';

    //if(s_shell_mode == MSH_MODE)
    {
        c_mode = ' ';
    }

    if(len < _CMDFUN_NAME_LENGTH_MAX)
    {
        for(i = 0; i < len; i++)
        {
            if(pbuf[i] != c_mode)
            {
                name[i] = pbuf[i];
            }
            else
            {
                name[i] = '\0';
                ret = i;
                i = len;
            }
        }
    }


    return ret;
}
/******************************************************************************************************
  * @brief  获取交互命令的参数
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
static uint32_t ConsoleGetFunArg(uint32_t* arg, uint8_t namelen, const uint8_t *pbuf, uint32_t len)
{
    uint8_t  arg_count                         = 0;      /*解析出来的参数个数*/
    uint8_t  ascii_count                       = 0;      /*解析出来的每个参数数值个数 928-> 3  1024->4*/
    uint32_t i                                      = 0;
    uint8_t arg_arr[_CMDFUN_ARG_NUM_MAX][10]  = {0};



    for(i = namelen + 1; i < len; i++)
    {

        if((pbuf[i] != ',') && (pbuf[i] != ' ') && (pbuf[i] != ')'))
        {
            arg_arr[arg_count][ascii_count]  = pbuf[i];
            ascii_count ++;
        }
        else
        {
            if((pbuf[i] == ',') || (pbuf[i] == ')'))
            {
//                for(j = 0; j < ascii_count; j++)
//                {
//                    arg[arg_count] += (arg_arr[arg_count][j] - 48) * pow(10, ascii_count - j - 1);
//                }
                arg[arg_count] = strtoul((char *)(arg_arr[arg_count]), NULL, 0);
                ascii_count = 0;
                arg_count++;
                if(pbuf[i] == ')')
                {
                    i = len;
                }
            }
        }

    }
    return arg_count;
}
/******************************************************************************************************
  * @brief  获取MSH交互命令的参数
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
static int32_t console_msh_get_arg(uint8_t *argv[_CMDFUN_ARG_NUM_MAX], uint8_t *cmd, uint32_t length)
{
    uint8_t *ptr;
    uint32_t position;
    uint32_t argc;
    uint32_t i;

    ptr = cmd;
    position = 0;
    argc = 0;

    while (position < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++;
            position ++;
        }

        if(argc >= _CMDFUN_ARG_NUM_MAX)
        {
            printf("Too many args ! We only Use:\n");
            for(i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
            break;
        }

        if (position >= length) break;

        /* handle string */
        if (*ptr == '"')
        {
            ptr ++;
            position ++;
            argv[argc] = ptr;
            argc ++;

            /* skip this string */
            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++;
                        position ++;
                    }
                }
                ptr ++;
                position ++;
            }
            if (position >= length) break;

            /* skip '"' */
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++;
                position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}
/******************************************************************************************************
  * @brief  从控制台命令中寻找用户调用的命令
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
static void* ConsoleFunctionLookUp(const char* name, uint8_t mode)
{
    _pconsole_cmd_t index = NULL;
    _pconsole_cmd_msh_t index_msh = NULL;

    if(mode == MSH_MODE)
    {
        for (index_msh = _spConsoleTableBeginMsh; index_msh < _spConsoleTableEndMsh; index_msh++)
        {
            if (strcmp((const char*)index_msh->name, name) == 0)
            {
                return index_msh;
            }
        }
    }
    else
    {
        for (index = _spConsoleTableBegin; index < _spConsoleTableEnd; index++)
        {
            if (strcmp((const char*)index->name, name) == 0)
            {
                return index;
            }
        }
    }

    return NULL;

}
/******************************************************************************************************
  * @brief  从控制台命令中寻找用户调用的命令
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
void ConsoleNameLookUp(const char* name, char* ret_name)
{

    _pconsole_cmd_msh_t msh_index = NULL;
    _pconsole_cmd_t index = NULL;
    uint32_t n_num = 0;

    _get_control_name();
    if(s_shell_mode == MSH_MODE)
    {
        for (msh_index = _spConsoleTableBeginMsh; msh_index < _spConsoleTableEndMsh; msh_index++)
        {
            if(strncmp((const char*)msh_index->name, name, strlen(name)) == 0)
            {
                if(n_num == 0)
                {
                    /*只有一个时直接把名字补全*/
                    printf("\r%s%s", p_name, msh_index->name);
                    strncpy(ret_name, msh_index->name, strlen(msh_index->name));

                }
                else if(n_num == 1)
                {
                    /*多于一个时把用户输入名字恢复显示*/
                    printf("\r%s%s%s\r\n", p_name, name, _CMDFUN_SPACE_X30);
                    printf("%s\r\n", ret_name);
                    printf("%s\r\n", msh_index->name);
                    ret_name[0] = 0;
                }
                else
                {
                    printf("%s\r\n", msh_index->name);
                }
                n_num++;
            }
        }
    }
    else
    {
        for (index = _spConsoleTableBegin; index < _spConsoleTableEnd; index++)
        {
            if(strncmp((const char*)index->name, name, strlen(name)) == 0)
            {
                if(n_num == 0)
                {
                    /*只有一个时直接把名字补全*/
                    printf("\r%s%s", p_name, index->name);
                    strncpy(ret_name, index->name, strlen(index->name));

                }
                else if(n_num == 1)
                {
                    /*多于一个时把用户输入名字恢复显示*/
                    printf("\r%s%s%s\r\n", p_name, name, _CMDFUN_SPACE_X30);
                    printf("%s\r\n", ret_name);
                    printf("%s\r\n", index->name);
                    ret_name[0] = 0;
                }
                else
                {
                    printf("%s\r\n", index->name);
                }
                n_num++;
            }
        }
    }

    if((n_num != 1) && (n_num != 0))
    {
        printf("%s%s", p_name, name);
    }
}

/******************************************************************************************************
  * @brief  控制台默认命令
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
static void list(void)
{
    _pconsole_cmd_t index = NULL;
    _pconsole_cmd_msh_t index_msh = NULL;

    if(s_shell_mode != MSH_MODE)
    {
        index = _spConsoleTableBegin;

        printf("--Function List:\r\n");
        for(; index < _spConsoleTableEnd; index++)
        {
            printf("  %-25s-- %s\r\n", index->name, index->des) ;
        }
    }
    else
    {
        index_msh = _spConsoleTableBeginMsh;

        printf("--Function List:\r\n");
        for(; index_msh < _spConsoleTableEndMsh; index_msh++)
        {
            printf("  %-25s-- %s\r\n", index_msh->name, index_msh->des) ;
        }
    }
}
/******************************************************************************************************
  * @brief  hello world
  * @param
  * @param
  * @retval
  *****************************************************************************************************/

static void FINSH(void)
{
    s_shell_mode = FINSH_MODE;
}
static void MSH(void)
{
    s_shell_mode = MSH_MODE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//                                  Exported Functions
//
///////////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************************************
  * @brief  控制台接收处理
  * @param
  * @param
  * @retval
  *****************************************************************************************************/
void console_recive_handler(const uint8_t * p_buffer, uint32_t n_length)
{
    static uint8_t sarr_cmd_name[_CMDFUN_CACHE_NUM_MAX][_CMDFUN_NAME_LENGTH_MAX] = {0};
    static uint8_t sarr_arrow_buff[3] = {0};
    static uint8_t sc_row    = 0;
    static uint8_t sc_history_row    = 0;
    static uint8_t sc_column = 0;
    static uint8_t sc_arrow_column = 0;

    _pconsole_cmd_t     user_cmd               = NULL;
    _pconsole_cmd_msh_t msh_user_cmd           = NULL;
    char cmd_name[_CMDFUN_NAME_LENGTH_MAX] = {0};

    uint8_t *argv[_CMDFUN_ARG_NUM_MAX] = {0};
    uint32_t cmd_arg[_CMDFUN_ARG_NUM_MAX]       = {0};

//    uint32_t name_len = 0;
    int32_t argc = 0;

    _get_control_name();
    if((sc_arrow_column == 0) && (p_buffer[0] != 0x1b))
    {
        /*不进入箭头判断模式*/
        if(((p_buffer[0] == '\r') && (n_length == 1)) ||
                ((p_buffer[0] == '\r') && (p_buffer[1] == 0) && (n_length == 2)))
        {
            if(sc_column == 0)
            {
                /*没有缓存字符，直接换行*/
                printf("\r\n%s", p_name);
            }
            else
            {
                /*查看是否需要命令回滚*/
                sc_row = sc_history_row;
                if(sc_row != sc_history_row)
                {
                    memcpy(sarr_cmd_name[sc_row], sarr_cmd_name[sc_history_row], _CMDFUN_NAME_LENGTH_MAX * sizeof(uint8_t));
                }

                /*缓存字符后，匹配指令*/
//                name_len =
                console_get_fun_name(cmd_name, sarr_cmd_name[sc_row], sc_column);

                if(s_shell_mode == MSH_MODE)
                {
                    msh_user_cmd = ConsoleFunctionLookUp(cmd_name, s_shell_mode);
                }
                else
                {
                    user_cmd = ConsoleFunctionLookUp(cmd_name, s_shell_mode);
                }
                if(((msh_user_cmd != NULL) && (s_shell_mode == MSH_MODE)) || \
                        ((user_cmd != NULL) && (s_shell_mode != MSH_MODE)))

                {
                    printf("\r\n");

                    if(s_shell_mode == MSH_MODE)
                    {

                        argc = console_msh_get_arg(argv, sarr_cmd_name[sc_row], sc_column);
                        msh_user_cmd->fun(argc, argv);
                    }
                    else
                    {
                        argc = console_msh_get_arg(argv, sarr_cmd_name[sc_row], sc_column);
                        if(*argv[1] > '9')
                        {
                            user_cmd->fun(argv[1]);
                        }
                        else
                        {
                            user_cmd->fun(atoi((char *)argv[1]),  atoi((char *)argv[2]),  atoi((char *)argv[3]), \
                                          atoi((char *)argv[4]),  atoi((char *)argv[5]),  atoi((char *)argv[6]), \
                                          atoi((char *)argv[7]),  atoi((char *)argv[8]),  atoi((char *)argv[9]), \
                                          atoi((char *)argv[10]), atoi((char *)argv[11]), atoi((char *)argv[12]), \
                                          atoi((char *)argv[13]), atoi((char *)argv[14]), atoi((char *)argv[15]), \
                                          atoi((char *)argv[16]), atoi((char *)argv[17]), atoi((char *)argv[18]), \
                                          atoi((char *)argv[19]), atoi((char *)argv[20]), atoi((char *)argv[21]), \
                                          atoi((char *)argv[22]), atoi((char *)argv[23]), atoi((char *)argv[24]));
                        }

                    }
                    printf("\r\n%s", p_name);
                }
                else
                {
                    printf("\r\n>>unknown\r\n");
                    printf("\r\n%s", p_name);
                }

                /*命令处理完后更新参数*/
                sc_column = 0;
                sc_row++;
                if(sc_row >= _CMDFUN_CACHE_NUM_MAX)
                {
                    sc_row = 0;
                }
                sc_history_row = sc_row;
                memset(sarr_cmd_name[sc_row], 0, _CMDFUN_NAME_LENGTH_MAX);
            }
        }
        else
        {
            if(p_buffer[0] == '\b')
            {
                /*退格符*/
                if(sc_column != 0)
                {
                    sc_row = sc_history_row;
                    sc_column--;
                    sarr_cmd_name[sc_row][sc_column] = '\0';
                    printf("\b \b ");
                    printf("\b \b");
                }

            }
            else if(p_buffer[0] == 0x09)
            {
                /*Table符*/
                sc_row = sc_history_row;
                ConsoleNameLookUp((const char *)&sarr_cmd_name[sc_row][0], cmd_name);
                if(cmd_name[0] != 0)
                {
                    memcpy(sarr_cmd_name[sc_row], cmd_name, _CMDFUN_NAME_LENGTH_MAX * sizeof(uint8_t));
                    sc_column = strlen(cmd_name);
                }

            }
            else if((sc_column + n_length) < _CMDFUN_NAME_LENGTH_MAX)
            {

                memcpy(&sarr_cmd_name[sc_row][sc_column], p_buffer, n_length);
                sc_column += n_length;
                printf("%s", p_buffer);
            }
            else
            {
                printf("\r\n>>input too long");
                printf("\r\n%s", p_name);
                sc_column = 0;
            }
        }
    }
    else
    {
        /*UP    ARROW sequence : <ESC>[A (1b,5b,41)
          DOWN  ARROW sequence : <ESC>[B (1b,5b,42)
          RIGHT ARROW sequence : <ESC>[C (1b,5b,43)
          LEFT  ARROW sequence : <ESC>[D (1b,5b,44) */
        if((sc_arrow_column + n_length) <= 3)
        {
            memcpy(&sarr_arrow_buff[sc_arrow_column], p_buffer, n_length);
            sc_arrow_column += n_length;
        }
        else
        {
            sc_arrow_column = 0;
            printf("\r\n%s", p_name);
        }

        if(sc_arrow_column == 3)
        {
            if(((sarr_arrow_buff[0] == 0x1b) && (sarr_arrow_buff[1] == 0x5b) && (sarr_arrow_buff[2] == 0x41)) ||
                    ((sarr_arrow_buff[0] == 0x1b) && (sarr_arrow_buff[1] == 0x5b) && (sarr_arrow_buff[2] == 0x42)))
            {

                do
                {
                    if((sarr_arrow_buff[0] == 0x1b) && (sarr_arrow_buff[1] == 0x5b) && (sarr_arrow_buff[2] == 0x41))
                    {
                        if(sc_history_row != 0)
                        {
                            sc_history_row--;
                        }
                        else
                        {
                            sc_history_row = _CMDFUN_CACHE_NUM_MAX - 1;
                        }
                    }
                    else
                    {
                        if(sc_history_row != (_CMDFUN_CACHE_NUM_MAX - 1))
                        {
                            sc_history_row++;
                        }
                        else
                        {
                            sc_history_row = 0;
                        }
                    }
                    if(sarr_cmd_name[sc_history_row][0] != 0)
                    {
                        break;
                    }

                }
                while(sc_history_row != sc_row);

                /*打印历史命令*/
                for(uint32_t i = 0; i < _CMDFUN_NAME_LENGTH_MAX; i++)
                {
                    if((sarr_cmd_name[sc_history_row][i] == 0) && (sarr_cmd_name[sc_history_row][i + 1] == 0))
                    {
                        break;
                    }
                    else if(sarr_cmd_name[sc_history_row][i] == 0)
                    {
                        sarr_cmd_name[sc_history_row][i] = ' ';
                    }

                }
                printf("\r%s\r%s%s", _CMDFUN_SPACE_X30, p_name, (char*)&sarr_cmd_name[sc_history_row]);
                sc_column = strlen((char *)&sarr_cmd_name[sc_history_row][0]);
            }
            else if((sarr_arrow_buff[0] == 0x1b) && (sarr_arrow_buff[1] == 0x5b) && (sarr_arrow_buff[2] == 0x43))
            {

            }
            else if((sarr_arrow_buff[0] == 0x1b) && (sarr_arrow_buff[1] == 0x5b) && (sarr_arrow_buff[2] == 0x44))
            {

            }
            sc_arrow_column = 0;
        }
    }
}
/******************************************************************************************************
  * @brief  初始化控制台控件
  * @param  控制台挂载的发送函数
  * @param
  * @retval
  *****************************************************************************************************/
void console_init(void)
{
    /* ARM C Compiler */
    extern const int FConTab$$Base;
    extern const int FConTab$$Limit;
    extern const int FConTabMsh$$Base;
    extern const int FConTabMsh$$Limit;

    ConsoleFunctioninit(&FConTab$$Base,    &FConTab$$Limit, FINSH_MODE);
    ConsoleFunctioninit(&FConTabMsh$$Base, &FConTabMsh$$Limit, MSH_MODE);
}


FINSH_CMD_EXPORT(MSH,   .num say hello to num);
FINSH_CMD_EXPORT(list,  list all console cmd);

MSH_CMD_EXPORT(list,  list all console cmd);
MSH_CMD_EXPORT(FINSH, .num say hello to num);