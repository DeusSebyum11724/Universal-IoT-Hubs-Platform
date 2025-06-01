#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"
#include "app_usbd.h"
#include "app_usbd_cdc_acm.h"
#include "app_error.h"
#include "nrf_drv_clock.h"

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            0, 1, 1,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event) {
    switch (event) {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
            app_usbd_cdc_acm_read(&m_app_cdc_acm, NULL, 0);
            break;
        default:
            break;
    }
}

int main(void) {
    APP_ERROR_CHECK(nrf_drv_clock_init());
    nrf_drv_clock_lfclk_request(NULL);
    APP_ERROR_CHECK(app_usbd_init(NULL));

    app_usbd_class_inst_t const * class_inst =
        app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    APP_ERROR_CHECK(app_usbd_class_append(class_inst));

    app_usbd_enable();
    app_usbd_start();

    bsp_board_init(BSP_INIT_LEDS);

    while (true) {
        while (app_usbd_event_queue_process()) {}
        app_usbd_cdc_acm_write(&m_app_cdc_acm, "Hello from nRF52840\r\n", 23);
        bsp_board_led_invert(0);
        nrf_delay_ms(1000);
    }
}
