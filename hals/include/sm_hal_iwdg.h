/*
 * sm_hal_iwdg.h
 *
 *  Created on: Jul 12, 2023
 *      Author: Admin
 */

#ifndef HAL_INCLUDE_SM_HAL_IWDG_H_
#define HAL_INCLUDE_SM_HAL_IWDG_H_

#include <stdint.h>
#include <stdio.h>


typedef struct sm_hal_iwdg sm_hal_iwdg_t;

typedef struct sm_hal_iwdg_proc{
    int32_t (*open)(sm_hal_iwdg_t *);
    int32_t (*close)(sm_hal_iwdg_t *);
    int32_t (*reset)(sm_hal_iwdg_t *);
}sm_hal_iwdg_proc_t;

struct sm_hal_iwdg{
    const sm_hal_iwdg_proc_t *proc;
    void *handle;
};

sm_hal_iwdg_t* sm_hal_iwdg_init(sm_hal_iwdg_proc_t *sm_proc, void* _handle);

static inline int32_t sm_hal_iwdg_open(sm_hal_iwdg_t *_this){
    return _this->proc->open(_this);
}
static inline int32_t sm_hal_iwdg_close(sm_hal_iwdg_t *_this){
    return _this->proc->close(_this);
}
static inline int32_t sm_hal_iwdg_reset(sm_hal_iwdg_t *_this){
    return _this->proc->reset(_this);
}

#endif /* hal_INCLUDE_SM_HAL_IWDG_H_ */
