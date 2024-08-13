#include "sm_hal_iic.h"
#include <stdio.h>
#include <stdlib.h>
#include "sm_hal_delay.h"

typedef struct sm_hal_iic_impl sm_hal_iic_impl_t;

struct sm_hal_iic_impl
{
    /* data */
    sm_hal_iic_t base;
    volatile uint8_t event;
};




sm_hal_iic_t* sm_hal_iic_init(sm_hal_iic_proc_t *_proc,void* _handle){
    sm_hal_iic_impl_t *parent = (sm_hal_iic_impl_t*) malloc(sizeof(sm_hal_iic_impl_t));


    parent->base.handle = _handle;
    parent->base.proc = _proc;

    return &parent->base;

}

void sm_hal_iic_deinit(sm_hal_iic_t *_this){


    sm_hal_iic_impl_t* parent = (sm_hal_iic_impl_t*) _this;

    if(_this->proc->close != NULL)
        _this->proc->close(_this);

    free(parent);

}

int32_t sm_hal_iic_callback(sm_hal_iic_t *_this, uint8_t event)
{
    sm_hal_iic_impl_t *parent = (sm_hal_iic_impl_t*) _this;
    parent->event = event;
    return 0;
}

int32_t sm_hal_iic_write(sm_hal_iic_t *_this, uint8_t *_data, uint32_t _len)

{
    sm_hal_iic_impl_t *parent = (sm_hal_iic_impl_t*) _this;
    parent->event = 0;
    parent->base.proc->write (_this, _data, _len);
    uint32_t timeout = 10;
    while (parent->event == 0)
    {
        if (timeout-- == 0)
            break;
        sm_hal_delay_ms (1);
    }
    if (parent->event != 0)
        return 0;
    return -1;
}

int32_t sm_hal_iic_read(sm_hal_iic_t *_this, uint8_t *_data, uint32_t _len)

{
    sm_hal_iic_impl_t *parent = (sm_hal_iic_impl_t*) _this;
    parent->event = 0;
    parent->base.proc->read (_this, _data, _len);
    uint32_t timeout = 10;
    while (timeout-- > 0)
    {
        if (parent->event != 0)
            return 0;
        sm_hal_delay_ms (1);
    }
    return -1;
}
