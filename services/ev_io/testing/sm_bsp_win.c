//
// Created by DELL 5425 on 3/4/2024.
//

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include "sm_bsp_win.h"
#include "../sm_sv_io.h"
//static sm_hal_io_t* g_in_key;
//static sm_hal_io_t* g_out_left_light;

#define printf(x,y,z,t) my_printf(x,y,z,t)
void my_printf(const char *x, uint8_t y, uint8_t z, uint8_t t) {
    (void*) x;
    (void) y;
    (void) z;
    (void) t;
}

uint8_t port_pin_value[16][16];
uint8_t port_pin_mode[16][16];

typedef struct io {
    uint8_t port;
    uint8_t pin;
}bsp_io_t;

static bsp_io_t IO[48];

typedef enum IO {
    IN_KEY,
    IN_LEFT,
    IN_RIGHT,
    IN_PHARE,
    IN_COS,
    IN_PARKING,
    IN_DRV_MODE,
    IN_REVERSE_MODE,
    IN_HORN,
    IN_LEFT_BREAK,
    IN_RIGHT_BREAK,
    OUT_LEFT,
    OUT_RIGHT,
    OUT_PHARE,
    OUT_COS,
    OUT_HORN,
    OUT_TAIL,
}BSP_IO;

int32_t sm_bsp_init(){
    for (uint8_t i=0; i<16; i++)
        for (uint8_t j=0; j<16; j++){
            port_pin_value[i][j] = 0;
            port_pin_mode[i][j] = 0;
        }
    for (uint8_t i=0; i<48; i++) {
        IO[i].pin = i%16;
        IO[i].port = i/16;
    }
    return 0;
}
uint8_t sm_bsp_io_get_value_key(){
    uint8_t temp_value = port_pin_value[IO[IN_KEY].port][IO[IN_KEY].pin];
    printf("IN_KEY | PORT %d PIN %d | %d\n", IO[IN_KEY].port, IO[IN_KEY].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_left_light(){
    uint8_t temp_value = port_pin_value[IO[IN_LEFT].port][IO[IN_LEFT].pin];
    printf("IN_LEFT | PORT %d PIN %d | %d\n", IO[IN_LEFT].port, IO[IN_LEFT].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_right_light(){
    uint8_t temp_value = port_pin_value[IO[IN_RIGHT].port][IO[IN_RIGHT].pin];
    printf("IN_RIGHT | PORT %d PIN %d | %d\n", IO[IN_RIGHT].port, IO[IN_RIGHT].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_phare_light(){
    uint8_t temp_value = port_pin_value[IO[IN_PHARE].port][IO[IN_PHARE].pin];
    printf("IN_PHARE | PORT %d PIN %d | %d\n", IO[IN_PHARE].port, IO[IN_PHARE].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_cos_light(){
    uint8_t temp_value = port_pin_value[IO[IN_COS].port][IO[IN_COS].pin];
    printf("IN_COS | PORT %d PIN %d | %d\n", IO[IN_COS].port, IO[IN_COS].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_parking(){
    uint8_t temp_value = port_pin_value[IO[IN_PARKING].port][IO[IN_PARKING].pin];
    printf("IN_PARKING | PORT %d PIN %d | %d\n", IO[IN_PARKING].port, IO[IN_PARKING].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_driving_mode(){
    uint8_t temp_value = port_pin_value[IO[IN_DRV_MODE].port][IO[IN_DRV_MODE].pin];
    printf("IN_DRV_MODE | PORT %d PIN %d | %d\n", IO[IN_DRV_MODE].port, IO[IN_DRV_MODE].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_left_break(){
    uint8_t temp_value = port_pin_value[IO[IN_LEFT_BREAK].port][IO[IN_LEFT_BREAK].pin];
    printf("IN_LEFT_BREAK | PORT %d PIN %d | %d\n", IO[IN_LEFT_BREAK].port, IO[IN_LEFT_BREAK].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_right_break(){
    uint8_t temp_value = port_pin_value[IO[IN_RIGHT_BREAK].port][IO[IN_RIGHT_BREAK].pin];
    printf("IN_RIGHT_BREAK | PORT %d PIN %d | %d\n", IO[IN_RIGHT_BREAK].port, IO[IN_RIGHT_BREAK].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_horn(){
    uint8_t temp_value = port_pin_value[IO[IN_HORN].port][IO[IN_HORN].pin];
    printf("IN_HORN | PORT %d PIN %d | %d\n", IO[IN_HORN].port, IO[IN_HORN].pin, temp_value);
    return temp_value;
}
uint8_t sm_bsp_io_get_value_reverse_mode(){
    uint8_t temp_value = port_pin_value[IO[IN_REVERSE_MODE].port][IO[IN_REVERSE_MODE].pin];
    printf("IN_REVERSE_MODE PORT %d PIN %d | %d\n", IO[IN_REVERSE_MODE].port, IO[IN_REVERSE_MODE].pin, temp_value);
    return temp_value;
}
int32_t sm_bsp_io_set_value_left_light(uint8_t _value){
    port_pin_value[IO[OUT_LEFT].port][IO[OUT_LEFT].pin] = _value;
    printf("SET OUT_LEFT | PORT %d PIN %d | %d\n", IO[OUT_LEFT].port, IO[OUT_LEFT].pin, _value);
    return 0;
}
int32_t sm_bsp_io_set_value_right_light(uint8_t _value){
    port_pin_value[IO[OUT_RIGHT].port][IO[OUT_RIGHT].pin] = _value;
    printf("SET OUT_RIGHT | PORT %d PIN %d | %d\n", IO[OUT_RIGHT].port, IO[OUT_RIGHT].pin, _value);
    return 0;
}
int32_t sm_bsp_io_set_value_phare_light(uint8_t _value){
    port_pin_value[IO[OUT_PHARE].port][IO[OUT_PHARE].pin] = _value;
    printf("SET OUT_PHARE | PORT %d PIN %d | %d\n", IO[OUT_PHARE].port, IO[OUT_PHARE].pin, _value);
    return 0;
}
int32_t sm_bsp_io_set_value_cos_light(uint8_t _value){
    port_pin_value[IO[OUT_COS].port][IO[OUT_COS].pin] = _value;
    printf("SET OUT_COS | PORT %d PIN %d | %d\n", IO[OUT_COS].port, IO[OUT_COS].pin, _value);
    return 0;
}
int32_t sm_bsp_io_set_value_tail_light(uint8_t _value){
    port_pin_value[IO[OUT_TAIL].port][IO[OUT_TAIL].pin] = _value;
    printf("SET OUT_TAIL | PORT %d PIN %d | %d\n", IO[OUT_TAIL].port, IO[OUT_TAIL].pin, _value);
    return 0;
}
int32_t sm_bsp_io_set_value_horn(uint8_t _value){
    port_pin_value[IO[OUT_HORN].port][IO[OUT_HORN].pin] = _value;
    printf("SET OUT_HORN | PORT %d PIN %d | %d\n", IO[OUT_HORN].port, IO[OUT_HORN].pin, _value);
    return 0;
}

