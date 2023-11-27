#include <zephyr/sys/reboot.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/scan.h>
#include "ble.h"
#include "usb.h"
#include "log.h"
#include "config.h"

LOG_MODULE_REGISTER(ble, LOG_LEVEL_INF);

#define TRY(...)                                        \
{                                                       \
    int err = __VA_ARGS__;                              \
    if (err) {                                          \
        LOG_E("failure: " #__VA_ARGS__ " -> %d", err);  \
        return;                                         \
    }                                                   \
}

namespace app {
namespace {

struct k_work_delayable work_scan_start;
struct k_work_delayable work_scan_stop;
bool scanning = false;

constexpr auto adv_type_str(uint8_t adv_type)
{
    switch (adv_type) {
        case BT_GAP_ADV_TYPE_ADV_IND:           return "scannable and connectable";
        case BT_GAP_ADV_TYPE_ADV_DIRECT_IND:    return "directed connectable";
        case BT_GAP_ADV_TYPE_ADV_SCAN_IND:      return "non-connectable and scannable";
        case BT_GAP_ADV_TYPE_ADV_NONCONN_IND:   return "non-connectable and non-scannable";
        case BT_GAP_ADV_TYPE_SCAN_RSP:          return "additional advertising data requested by an active scanner";
        case BT_GAP_ADV_TYPE_EXT_ADV:           return "extended";
        default: return "<unknown>";
    }
}

constexpr auto phy_str(uint8_t phy)
{
    switch (phy) {
        case 0:                     return "no packets";
        case BT_GAP_LE_PHY_1M:      return "LE 1M";
        case BT_GAP_LE_PHY_2M:      return "LE 2M";
        case BT_GAP_LE_PHY_CODED:   return "LE Coded";
        default: return "<unknown>";
    }
}

void log_scan_info(struct bt_scan_device_info *info, bool connectable)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(info->recv_info->addr, addr, sizeof(addr));

    LOG_I("%s   \n\
        rssi:           %+d     \n\
        tx_power:       %+d     \n\
        sid:            %u      \n\
        interval:       %u ms   \n\
        adv_type:       %s      \n\
        primary_phy:    %s      \n\
        secondary_phy:  %s      \n\
        connectable:    %s",
        addr, 
        info->recv_info->rssi,
        info->recv_info->tx_power,
        info->recv_info->sid,
        info->recv_info->interval * 5 / 4,
        adv_type_str(info->recv_info->adv_type),
        phy_str(info->recv_info->primary_phy),
        phy_str(info->recv_info->secondary_phy),
        connectable ? "yes" : "no");
}

void scan_process(struct bt_scan_device_info *device_info)
{
    uint8_t* raw_data   = device_info->adv_data->data;
    uint16_t raw_len    = device_info->adv_data->len;
    LOG_HEX_D(raw_data, raw_len, "adv_data");
    usb_send(raw_data, raw_len);

    // if (raw_len > 7) {
    //     uint8_t payload_len = raw_data[3];
    //     if (payload_len == raw_len - 4)
    //         capsule_data_check(&raw_data[5], payload_len - 1, device_info->recv_info->addr->a.val, device_info->recv_info->rssi);
    // }
}

void scan_filter_match(struct bt_scan_device_info *device_info, struct bt_scan_filter_match *filter_match, bool connectable)
{
    LOG_D(TXT_GRN "[MATCHED]");
    log_scan_info(device_info, connectable);
    scan_process(device_info);
}

void scan_filter_mismatch(struct bt_scan_device_info *device_info, bool connectable)
{
    LOG_D(TXT_RED "[NOT MATCHED]"); 
    log_scan_info(device_info, connectable);
    scan_process(device_info);
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, scan_filter_mismatch, nullptr, nullptr);

void scan_init()
{
    // Use active scanning and disable duplicate filtering to handle any
    // devices that might update their advertising data at runtime.
    struct bt_le_scan_param scan_param = {
        .type       = BT_LE_SCAN_TYPE_PASSIVE,
#if (BLE_LONG_RANGE_SCAN)
        .options    = BT_LE_SCAN_OPT_CODED | BT_LE_SCAN_OPT_NO_1M,
#else
        .options    = BT_LE_SCAN_OPT_NONE, 
#endif
        .interval   = BT_GAP_SCAN_FAST_INTERVAL,
        .window     = 0x60, // NOTE: Seems like 0x60 is maximum.
    };

    struct bt_scan_init_param scan_init = {
        .scan_param         = &scan_param,
        .connect_if_match   = 0,
        .conn_param         = nullptr,
    };

    bt_scan_init(&scan_init);
    bt_scan_cb_register(&scan_cb);

    LOG_D("scanning initialized");
}

void scan_start(struct k_work *item)
{
    if (scanning == false) {
        scanning = true;
        TRY(bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE));
        LOG_I("scanning started");
    } else {
        LOG_W("already started scanning");
    }
}

void scan_stop(struct k_work *item)
{
    if (scanning) {
        scanning = false;
        TRY(bt_scan_stop());
        LOG_I("scanning stopped");
    } else {
        LOG_W("already stopped scanning");
    }
}

}

void ble_init()
{
    int err = bt_enable(nullptr);
    if (err) {
        LOG_E("failure: bt_enable() -> %d", err);
        sys_reboot(SYS_REBOOT_COLD);
    } 
    scan_init();
    
    k_work_init_delayable(&work_scan_start, scan_start);
    k_work_init_delayable(&work_scan_stop, scan_stop);
}

void ble_uninit()
{
    k_work_cancel_delayable(&work_scan_start);
    k_work_cancel_delayable(&work_scan_stop);
    scan_stop(nullptr);
}

void ble_scan_start()
{
    k_work_cancel_delayable(&work_scan_start);
    scan_start(nullptr);
}

void ble_scan_stop()
{
    k_work_cancel_delayable(&work_scan_stop);
    scan_stop(nullptr);
}

}