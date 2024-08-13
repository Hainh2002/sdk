#ifndef SM_HAL_IO_H
#define SM_HAL_IO_H

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>
/**
 * @enum
 * @brief
 *
 */
typedef enum {
	SM_HAL_IO_OFF = 0,
	SM_HAL_IO_ON
} sm_hal_io_value_t;


typedef enum {
	SM_HAL_IO_INPUT = 0,
	SM_HAL_IO_OUTPUT
} sm_hal_io_mode_t;
typedef struct sm_hal_io sm_hal_io_t;
/**
 * @struct sm_hal_io_proc
 * @brief
 *
 */
typedef struct sm_hal_io_proc
{
    int32_t (*open)(sm_hal_io_t *_this, sm_hal_io_mode_t _mode);
    int32_t (*close)(sm_hal_io_t *_this);
    int32_t (*set_value)(sm_hal_io_t*, uint8_t);
    uint8_t (*get_value)(sm_hal_io_t*);
} sm_hal_io_proc_t;
/**
 * @struct sm_hal_io
 * @brief
 *
 */

struct sm_hal_io {
	sm_hal_io_proc_t *proc;
	void *handle; // gpio_port
	uint16_t pin;
	sm_hal_io_mode_t mode;
};
/**
 * @fn sm_hal_io_t sm_hal_io_init*(sm_hal_io_proc_t*, void*, uint16_t)
 * @brief
 *
 * @param m_proc
 * @param handle
 * @param pin
 * @return
 */
sm_hal_io_t* sm_hal_io_init(sm_hal_io_proc_t *_proc, void *_handle,
		uint16_t _pin);
/**
 * @fn void sm_hal_io_deinit(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 */
void sm_hal_io_deinit(sm_hal_io_t *_this);
/**
 * @fn int32_t sm_hal_io_set_value(sm_hal_io_t*, uint8_t)
 * @brief
 *
 * @param _this
 * @param level
 * @return
 */
int32_t sm_hal_io_set_value(sm_hal_io_t *_this, uint8_t _level);
/**
 * @fn uint8_t sm_hal_io_get_value(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 * @return
 */
uint8_t sm_hal_io_get_value(sm_hal_io_t *_this);
/**
 * @fn int32_t sm_hal_io_open(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_io_open(sm_hal_io_t *_this, sm_hal_io_mode_t _mode);
/**
 * @fn int32_t sm_hal_io_close(sm_hal_io_t*)
 * @brief
 *
 * @param _this
 * @return
 */
int32_t sm_hal_io_close(sm_hal_io_t *_this);

#ifdef __cplusplus
};
#endif

#endif //SM_hal_IO_H
