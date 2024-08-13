/*
 * sm_hal_iwdg.c
 *
 *  Created on: Jul 12, 2023
 *      Author: Admin
 */


#include "sm_hal_iwdg.h"
#include <stdlib.h>

sm_hal_iwdg_t* sm_hal_iwdg_init(sm_hal_iwdg_proc_t *_proc,void* _handle){
    sm_hal_iwdg_t *_this = (sm_hal_iwdg_t*) malloc(sizeof(sm_hal_iwdg_t));

    _this->handle = _handle;
    _this->proc = _proc;
    return _this;
}
