#include "zboss_api.h"
#include "zb_mem_config_med.h"
#include "zb_error_handler.h"
#include "zb_nrf52_internal.h"
#include "app_uart.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"

#define UART_TX_PIN 17
#define UART_RX_PIN 15

// UART init
void uart_init(void)
{
    const app_uart_comm_params_t params = {
        .rx_pin_no = UART_RX_PIN,
        .tx_pin_no = UART_TX_PIN,
        .rts_pin_no = UART_PIN_DISCONNECTED,
        .cts_pin_no = UART_PIN_DISCONNECTED,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity = false,
        .baud_rate = UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_ERROR_CHECK(app_uart_init(&params, NULL, NULL, APP_IRQ_PRIORITY_LOWEST));
}

// Transmit text over UART
void uart_send_str(const char *str)
{
    while (*str) {
        app_uart_put(*str++);
    }
}

// Callback on ZLL device detection
void device_found_callback(zb_zdo_match_desc_resp_hdr_t *resp)
{
    char buffer[64];
    uint16_t addr = resp->nwk_addr;
    snprintf(buffer, sizeof(buffer), "Device found: NWK=0x%04X\n", addr);
    uart_send_str(buffer);
}

// Zigbee logic
void zboss_startup_complete(zb_ret_t status)
{
    if (status == RET_OK) {
        uart_send_str("Zigbee coordinator started.\n");

        // Broadcast Match_Desc_req to find devices with ZLL profile
        zb_zdo_match_desc_param_t *req = ZB_BUF_GET_PARAM(ZB_GET_OUT_BUF(), zb_zdo_match_desc_param_t);
        req->nwk_addr = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
        req->addr_of_interest = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
        req->profile_id = 0xc05e;  // Zigbee Light Link profile ID

        req->num_in_clusters = 1;
        req->num_out_clusters = 0;
        req->cluster_list[0] = 0x0006;  // On/Off cluster

        ZB_ZDO_MATCH_DESC_REQ(ZB_GET_OUT_BUF(), req, device_found_callback);
    } else {
        uart_send_str("Zigbee start FAILED\n");
    }
}

int main(void)
{
    uart_init();
    uart_send_str("UART init done.\n");

    // Init Zigbee as Coordinator
    ZB_INIT("zll_coordinator");
    zb_set_network_role(ZB_NWK_DEVICE_TYPE_COORDINATOR);
    zb_set_max_children(10);

    zb_set_pan_id(0x1AAA);
    zb_set_rx_on_when_idle(ZB_TRUE);

    ZB_BDB_SET_PRIMARY_CHANNELS((1l << 11)); // Channel 11 for example

    ZB_BDB_START();

    zb_err_code_t err_code = zboss_start_no_autostart();
    ZB_ERROR_CHECK(err_code);

    ZB_SCHEDULE_CALLBACK(zboss_startup_complete, 0);

    while (1) {
        zboss_main_loop_iteration();
    }

    return 0;
}
