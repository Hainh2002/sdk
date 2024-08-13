#include "sm_hal_spi.h"
#include <stdio.h>
#include <stdlib.h>
typedef struct
{
    /* data */
    sm_hal_spi_t base;
}sm_hal_spi_impl_t;


sm_hal_spi_t* sm_hal_spi_init(sm_hal_spi_proc_t *_proc,void *_handle){
    sm_hal_spi_impl_t* parent = (sm_hal_spi_impl_t*) malloc(sizeof(sm_hal_spi_impl_t));
    parent->base.proc = _proc;
    parent->base.handle = _handle;
    return &parent->base;
}

void sm_hal_spi_deinit(sm_hal_spi_t* _this){
    sm_hal_spi_impl_t* parent = ( sm_hal_spi_impl_t*)_this;
    if(_this->proc->close != NULL)
        _this->proc->close(_this);
    free(parent);
}
