
/*
 * sp_hal_spi.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_SPI_H_
#define SM_HAL_SPI_H_

#include <stdint.h>

typedef void sm_hal_spi_t;

/**
 * @fn sm_hal_spi_t sm_hal_spi_init*(sm_hal_spi_proc_t*, void*)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @return
 */
sm_hal_spi_t* sm_hal_spi_init(void* _channel);
/**
 * @fn void sm_hal_spi_deinit(sm_hal_spi_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_spi_deinit(sm_hal_spi_t *_this);
/**
 * @fn int32_t sm_hal_spi_read(sm_hal_spi_t*, uint8_t*, uint16_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
int32_t sm_hal_spi_read(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len);
/**
 * @fn int32_t sm_hal_spi_write(sm_hal_spi_t*, uint8_t*, uint16_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
int32_t sm_hal_spi_write(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len);
/**
 * @fn int32_t sm_hal_spi_write_read(sm_hal_spi_t*, uint8_t*, uint8_t*, uint16_t)
 * @brief
 *
 * @param _this
 * @param src
 * @param dest
 * @param len
 * @return
 */
int32_t sm_hal_spi_write_read(sm_hal_spi_t *_this, uint8_t *_src, uint8_t *_dest, uint16_t _len);

#endif /* SM_HAL_SPI_H_ */
