#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include "usb.h"
#include "log.h"

LOG_MODULE_REGISTER(app_usb, LOG_LEVEL_INF);

namespace app {
namespace {

const struct device *dev;

}

void usb_init()
{
    dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

    if (!device_is_ready(dev)) {
        LOG_E("failure: CDC ACM device not ready");
        return;
    }

    int ret = usb_enable(nullptr);
    if (ret != 0) {
        LOG_E("failure: usb_enable() -> %d", ret);
        return;
    }
    LOG_I("Wait for DTR");

    uint32_t baudrate = 0;
    uint32_t dtr = 0;

    while (true) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        if (dtr)
            break;
        else
            k_sleep(K_MSEC(100)); /* Give CPU resources to low priority threads. */
    }
    LOG_I("DTR set");

    /* They are optional, we use them to test the interrupt endpoint */
    ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
    if (ret)
        LOG_W("failure: set DCD, uart_line_ctrl_set() -> %d", ret);

    ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
    if (ret)
        LOG_W("failure: set DSR, uart_line_ctrl_set() -> %d", ret);

    /* Wait 1 sec for the host to do all settings */
    k_busy_wait(1000000);

    ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
    if (ret)
        LOG_W("failure: get baudrate, uart_line_ctrl_get() -> %d", ret);
    else
        LOG_I("baudrate: %d", baudrate);

    // TODO
}

void usb_send(const uint8_t* data, size_t size)
{
    // TODO
}

}