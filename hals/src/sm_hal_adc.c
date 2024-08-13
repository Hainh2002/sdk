#include <sm_hal_adc.h>
#include <stdlib.h>

typedef struct 
{
    /* data */
    sm_hal_adc_t base;
    uint16_t *value;
    uint16_t *channel;
    void (*callback)(sm_hal_adc_t *_this,uint16_t value);
}sm_hal_adc_impl_t;


static void adc_callback(sm_hal_adc_t *_this,uint16_t value);

sm_hal_adc_t* sm_hal_adc_init(sm_hal_adc_proc_t *_proc, void *_handle, uint8_t _num_channel)
{

    sm_hal_adc_impl_t* parent = malloc(sizeof(sm_hal_adc_impl_t));

    parent->base.proc = _proc;
    parent->base.handle = _handle;
    parent->value = calloc (0, _num_channel);
    parent->channel = calloc (0, _num_channel);
    parent->callback = adc_callback;
    return &parent->base;
}

void sm_hal_adc_deinit(sm_hal_adc_t *_this){
    sm_hal_adc_impl_t* parent = (sm_hal_adc_impl_t*) _this;
    if(_this->proc->close != NULL)
        _this->proc->close(_this);
    free(parent);
}

static void adc_callback(sm_hal_adc_t *_this,uint16_t _value){
    sm_hal_adc_impl_t* parent = (sm_hal_adc_impl_t*) _this;
    parent->value[0] = _value;
}

uint16_t sm_hal_adc_read(sm_hal_adc_t *_this, uint8_t _channel)
{
//    sm_hal_adc_impl_t* parent = (sm_hal_adc_impl_t*) _this;
    return _this->proc->read(_this,_channel);;
}
void sm_hal_adc_callback(sm_hal_adc_t *_this,uint16_t _value){
    sm_hal_adc_impl_t* parent = (sm_hal_adc_impl_t*) _this;
    parent->callback(_this,_value);
}
int32_t sm_hal_adc_open(sm_hal_adc_t *_this){
    return _this->proc->open(_this);
}
int32_t sm_hal_adc_close(sm_hal_adc_t *_this){
	return _this->proc->close(_this);
}
