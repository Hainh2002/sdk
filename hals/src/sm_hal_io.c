
#include "sm_hal_io.h"
#include <stdio.h>
#include <stdlib.h>

sm_hal_io_t* sm_hal_io_init(sm_hal_io_proc_t *_proc,void *_handle,uint16_t _pin){
    sm_hal_io_t *_this = malloc(sizeof(sm_hal_io_t));

    _this->proc = _proc;
    _this->handle = _handle;
    _this->pin = _pin;
    return _this;
}

void sm_hal_io_deinit(sm_hal_io_t* _this){
    if(_this->proc->close != NULL)
        _this->proc->close(_this);
    free(_this);
}

int32_t sm_hal_io_set_value(sm_hal_io_t* _this, uint8_t _level){
    return _this->proc->set_value(_this,_level);
}
uint8_t sm_hal_io_get_value(sm_hal_io_t* _this){
    return _this->proc->get_value(_this);
}
int32_t sm_hal_io_open(sm_hal_io_t *_this, sm_hal_io_mode_t _mode) {
	return _this->proc->open(_this, _mode);
}
int32_t sm_hal_io_close(sm_hal_io_t *_this){
    return _this->proc->close(_this);
}
