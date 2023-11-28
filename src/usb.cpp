#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/ring_buffer.h>
#include "usb.h"
#include "log.h"
#include "config.h"
#if (USB_COBS)
#include <nth/nth.h>
#endif

LOG_MODULE_REGISTER(usb, LOG_LEVEL_DBG);

namespace app {
namespace {

#if (USB_ECHO_TEST)
constexpr auto ring_buf_size = 1024;

uint8_t ring_buffer[ring_buf_size];
struct ring_buf ringbuf;
#endif
const struct device* dev;
#if (USB_COBS)
nth::cobs_pipe_encoder cobs_pipe;
#endif

#if (USB_ECHO_TEST)

void interrupt_handler(const struct device *dev, void *user_data)
{
	ARG_UNUSED(user_data);

    while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {

        if (uart_irq_rx_ready(dev)) {

            int recv_len;
            int rb_len;
            uint8_t buffer[64];
            size_t len = MIN(ring_buf_space_get(&ringbuf), sizeof(buffer));
            recv_len = uart_fifo_read(dev, buffer, len);

            if (recv_len < 0) {
                recv_len = 0;
                LOG_E("Failed to read UART FIFO");
            }
            rb_len = ring_buf_put(&ringbuf, buffer, recv_len);

            if (rb_len < recv_len)
                LOG_E("Drop %u bytes", recv_len - rb_len);

            LOG_D("tty fifo -> ringbuf %d bytes", rb_len);

            if (rb_len)
                uart_irq_tx_enable(dev);
        }

        if (uart_irq_tx_ready(dev)) {

            uint8_t buffer[64];

            int rb_len = ring_buf_get(&ringbuf, buffer, sizeof(buffer));
            if (!rb_len) {
                LOG_D("Ring buffer empty, disable TX IRQ");
                uart_irq_tx_disable(dev);
                continue;
            }
            int send_len = uart_fifo_fill(dev, buffer, rb_len);
            if (send_len < rb_len) 
                LOG_E("Drop %d bytes", rb_len - send_len);

            LOG_D("ringbuf -> tty fifo %d bytes", send_len);
		}
	}
}

#endif

#if (USB_STATUS_CALLBACK)

void usb_status_handler(enum usb_dc_status_code cb_status, const uint8_t* param)
{

}

#endif

#if (USB_COBS)

size_t cobs_write_handler(const uint8_t* data, size_t size)
{
    LOG_HEX_I(data, size, TXT_MAG "USB TX COBS chunk");
    return size_t(uart_fifo_fill(dev, data, size));
}

#endif

}

void usb_init()
{
#if (USB_ECHO_TEST)
    ring_buf_init(&ringbuf, sizeof(ring_buffer), ring_buffer);
#endif
    dev = DEVICE_DT_GET_ONE(zephyr_cdc_acm_uart);

    if (!device_is_ready(dev)) {
        LOG_E("failure: CDC ACM device not ready");
        return;
    }
#if (USB_STATUS_CALLBACK)
    int ret = usb_enable(usb_status_handler);
#else
    int ret = usb_enable(nullptr);
#endif
    if (ret != 0) {
        LOG_E("failure: usb_enable() -> %d", ret);
        return;
    }
#if (USB_ECHO_TEST)

    LOG_I("Wait for DTR");

    uint32_t dtr = 0;

    while (true) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        if (dtr)
            break;
        else
            k_msleep(100); /* Give CPU resources to low priority threads. */
    }
    LOG_I("DTR set");

    /* They are optional, we use them to test the interrupt endpoint */
    ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DCD, 1);
    if (ret)
        LOG_W("failure: set DCD, uart_line_ctrl_set() -> %d", ret);

    ret = uart_line_ctrl_set(dev, UART_LINE_CTRL_DSR, 1);
    if (ret)
        LOG_W("failure: set DSR, uart_line_ctrl_set() -> %d", ret);

    k_msleep(1000); // Wait 1 sec for the host to do all settings

    uint32_t baudrate = 0;

    ret = uart_line_ctrl_get(dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
    if (ret)
        LOG_W("failure: get baudrate, uart_line_ctrl_get() -> %d", ret);
    else
        LOG_I("baudrate: %d", baudrate);

    uart_irq_callback_set(dev, interrupt_handler);
	uart_irq_rx_enable(dev);
#endif
}

bool usb_send(const void* data, size_t size)
{
    LOG_HEX_I(data, size, "USB TX raw chunk");
#if (USB_COBS)
    cobs_pipe.sink({static_cast<const uint8_t*>(data), size}, cobs_write_handler);
    return true;
#else
    uint8_t len = size;
    return  uart_fifo_fill(dev, &len, 1) == 1 &&
            uart_fifo_fill(dev, data, size) == int(size);
#endif
}

bool usb_send_finalize()
{
#if (USB_COBS)
    cobs_pipe.stop(cobs_write_handler);
    return true;
#else
    static_assert(false);
#endif
}

}