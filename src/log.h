#ifndef LOGGER_H
#define LOGGER_H

#include <zephyr/logging/log.h>
#include <SEGGER_RTT.h>

#ifdef __cplusplus 
extern "C" {
#endif

#define TXT_CLR     RTT_CTRL_CLEAR

#define TXT_RED     RTT_CTRL_TEXT_BRIGHT_RED
#define TXT_CYN     RTT_CTRL_TEXT_BRIGHT_CYAN
#define TXT_BLU     RTT_CTRL_TEXT_BRIGHT_BLUE
#define TXT_GRN     RTT_CTRL_TEXT_BRIGHT_GREEN
#define TXT_WHT     RTT_CTRL_TEXT_BRIGHT_WHITE
#define TXT_YEL     RTT_CTRL_TEXT_BRIGHT_YELLOW
#define TXT_MAG     RTT_CTRL_TEXT_BRIGHT_MAGENTA

#define LOG_I(fmt, ...)     LOG_INF(TXT_CYN fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...)     LOG_DBG(TXT_BLU fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...)     LOG_WRN(fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...)     LOG_ERR(fmt, ##__VA_ARGS__)

#define LOG_GRN(fmt, ...)   LOG_INF(TXT_GRN fmt, ##__VA_ARGS__)
#define LOG_RED(fmt, ...)   LOG_INF(TXT_RED fmt, ##__VA_ARGS__)

#define LOG_HEX_I(data, len, str)   LOG_HEXDUMP_INF(data, len, TXT_CYN str)
#define LOG_HEX_D(data, len, str)   LOG_HEXDUMP_DBG(data, len, TXT_BLU str)
#define LOG_HEX_W(data, len, str)   LOG_HEXDUMP_WRN(data, len, str)
#define LOG_HEX_E(data, len, str)   LOG_HEXDUMP_ERR(data, len, str)

#ifdef __cplusplus 
}
#endif

#endif