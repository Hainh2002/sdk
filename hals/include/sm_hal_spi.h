/*
 * sp_hal_spi.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef SM_HAL_SPI_H_
#define SM_HAL_SPI_H_

#include <stdint.h>
#include <stdio.h>

typedef struct sm_hal_spi sm_hal_spi_t;
/**
 * @struct sm_hal_spi_proc
 * @brief
 *
 */
typedef struct sm_hal_spi_proc
{
    /* data */
    int32_t (*read)(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len);
    int32_t (*write)(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len);
    int32_t (*write_read)(sm_hal_spi_t *_this, uint8_t *_src, uint8_t *_dest, uint16_t _len);
    int32_t (*open)(sm_hal_spi_t *_this);
    int32_t (*close)(sm_hal_spi_t *_this);
} sm_hal_spi_proc_t;
/**
 * @struct sm_hal_spi
 * @brief
 *
 */
struct sm_hal_spi
{
    /* data */
    const sm_hal_spi_proc_t *proc;
    void *handle;
};
/**
 * @fn sm_hal_spi_t sm_hal_spi_init*(sm_hal_spi_proc_t*, void*)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @return
 */
sm_hal_spi_t* sm_hal_spi_init(sm_hal_spi_proc_t *_proc, void *_handle);
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
static inline int32_t sm_hal_spi_read(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len)
{
    return _this->proc->read (_this, _buff, _len);
}
/**
 * @fn int32_t sm_hal_spi_write(sm_hal_spi_t*, uint8_t*, uint16_t)
 * @brief
 *
 * @param _this
 * @param buff
 * @param len
 * @return
 */
static inline int32_t sm_hal_spi_write(sm_hal_spi_t *_this, uint8_t *_buff, uint16_t _len)
{
    return _this->proc->write (_this, _buff, _len);
}
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
static inline int32_t sm_hal_spi_write_read(sm_hal_spi_t *_this, uint8_t *_src, uint8_t *_dest, uint16_t _len)
{
    return _this->proc->write_read (_this, _src, _dest, _len);
}
/**
 * @fn int32_t sm_hal_spi_open(sm_hal_spi_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_spi_open(sm_hal_spi_t *_this)
{
    return _this->proc->open (_this);
}
/**
 * @fn int32_t sm_hal_spi_close(sm_hal_spi_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_spi_close(sm_hal_spi_t *_this)
{
    return _this->proc->close (_this);
}

#endif /* hal_INCLUDE_SM_HAL_SPI_H_ */
