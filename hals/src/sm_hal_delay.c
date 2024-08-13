#include "sm_hal_delay.h"


#if RTOS
<<<<<<< HEAD
/**
 * @fn void sm_hal_delay_ms(uint32_t)
 * @brief
 *
 * @param timems
 */
void sm_hal_delay_ms(uint32_t timems){
    vTaskDelay(timems);
}
/**
 * @fn void sm_hal_delay_us(uint32_t)
 * @brief
 *
 * @param timeus
 */
void sm_hal_delay_us(uint32_t timeus){

uint32_t get_tick_count(){
    return 0;
}
#endif
