/*
 * sm_hal_flash.h
 *
 *  Created on: Jul 5, 2023
 *      Author: Admin
 */

#ifndef HAL_INCLUDE_SM_HAL_FLASH_H_
#define HAL_INCLUDE_SM_HAL_FLASH_H_

#include <stdint.h>
#include <stdio.h>
typedef struct sm_hal_flash sm_hal_flash_t;
/**
 * @struct sm_hal_flash_proc
 * @brief
 *
 */
typedef struct sm_hal_flash_proc
{
    /* data */
    int32_t (*write)(sm_hal_flash_t *_this, uint32_t _addr, void *_data, size_t _size);
    int32_t (*read)(sm_hal_flash_t *_this, uint32_t _addr, void *_data, size_t _size);
    int32_t (*erase)(sm_hal_flash_t *_this, uint32_t _addr, size_t _size);
    int32_t (*open)(sm_hal_flash_t *_this);
    int32_t (*close)(sm_hal_flash_t *_this);
} sm_hal_flash_proc_t;

/**
 * @struct sm_hal_flash
 * @brief
 *
 */
struct sm_hal_flash
{
    /* data */
    sm_hal_flash_proc_t *proc;
    void *handle;
};
/**
 * @fn sm_hal_flash_t sm_hal_flash_init*(sm_hal_flash_proc_t*, void*)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @return
 */
sm_hal_flash_t* sm_hal_flash_init(sm_hal_flash_proc_t *_proc, void *_handle);
/**
 * @fn void sm_hal_flash_deinit(sm_hal_flash_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_flash_deinit(sm_hal_flash_t *_this);
/**
 * @fn int32_t sm_hal_flash_write(sm_hal_flash_t*, uint32_t, void*, size_t)
 * @brief
 *
 * @param _this
 * @param addr
 * @param data
 * @param size
 * @return
 */
static inline int32_t sm_hal_flash_write(sm_hal_flash_t *_this, uint32_t _addr, void *_data, size_t _size)
{
    return _this->proc->write (_this, _addr, _data, _size);
}
/**
 * @fn int32_t sm_hal_flash_read(sm_hal_flash_t*, uint32_t, void*, size_t)
 * @brief
 *
 * @param _this
 * @param addr
 * @param data
 * @param size
 * @return
 */
static inline int32_t sm_hal_flash_read(sm_hal_flash_t *_this, uint32_t _addr, void *_data, size_t _size)
{
    return _this->proc->read (_this, _addr, _data, _size);
}
/**
 * @fn int32_t sm_hal_flash_erase(sm_hal_flash_t*, uint32_t, size_t)
 * @brief
 *
 * @param _this
 * @param addr
 * @param size
 * @return
 */
static inline int32_t sm_hal_flash_erase(sm_hal_flash_t *_this, uint32_t _addr, size_t _size)
{
    return _this->proc->erase (_this, _addr, _size);
}
/**
 * @fn int32_t sm_hal_flash_open(sm_hal_flash_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_flash_open(sm_hal_flash_t *_this)
{
    return _this->proc->open (_this);
}
/**
 * @fn int32_t sm_hal_flash_close(sm_hal_flash_t*)
 * @brief
 *
 * @param _this
 * @return
 */
static inline int32_t sm_hal_flash_close(sm_hal_flash_t *_this)
{
    return _this->proc->close (_this);
}

#endif /* hal_INCLUDE_SM_HAL_FLASH_H_ */
