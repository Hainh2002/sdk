#ifndef SV_LED_SEQ_H_
#define SV_LED_SEQ_H_

#include "sm_hal.h"
#include "sm_elapsed_timer.h"

#define SV_LED_BLINK_TIME_MS   500 //ms

typedef enum {
    OFF=0,
    ON,
}LED_ST;

typedef struct {
    sm_hal_io_t             *driver;
    LED_ST                  state;
    elapsed_timer_t         on_time;
    elapsed_timer_t         off_time;

    elapsed_timer_t         blink_time;
    uint8_t                 repeate_blink;
}led_t;

typedef struct {
    led_t  leds[3];
    struct period_cfg{
        uint16_t     green;
        uint16_t     yellow;
        uint16_t     red;
    }period;
    uint8_t     start_led;
}sv_led_seq_t;

int8_t 




#endif