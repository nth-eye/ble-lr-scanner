#include <zephyr/kernel.h>
#include "ble.h"
#include "usb.h"
#include "log.h"
#include "config.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

void regout0()
{
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) == (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) | (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NVIC_SystemReset();
    }
}

void approtect_hw_disable(void)
{
    if ((NRF_UICR->APPROTECT & UICR_APPROTECT_PALL_Msk) == (UICR_APPROTECT_PALL_Msk))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_UICR->APPROTECT = (UICR_APPROTECT_PALL_HwDisabled << UICR_APPROTECT_PALL_Pos);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {}
        NVIC_SystemReset();
    }  
}

int main(void)
{
    regout0();
    approtect_hw_disable();
    app::usb_init();
    app::ble_init();
    app::ble_scan_start();
#if (USB_COBS_TEST)
    uint8_t buf[27];
    int cnt = 0;
    while (1) {
        for (int i = 0; i < sizeof(buf); ++i)
            buf[i] = cnt * i;
        cnt++;
        app::usb_send(buf, sizeof(buf));
        k_msleep(1000);
    }
#endif
}
