#include <sm_hal_timer.h>
#include <stdlib.h>
typedef struct sm_hal_timer_impl
{
    /* data */
    sm_hal_timer_t base;
    void* arg;
    const char *name;
    void (*callback)(sm_hal_timer_t *_this); // Calling funtion in timer interrupt
}sm_hal_timer_impl_t;

static void timer_callback(sm_hal_timer_t *_this);

sm_hal_timer_t* sm_hal_timer_init(sm_hal_timer_proc_t *_proc,timerfuntion_t _funtion,const char* _name,void* _arg,void *_handle){

    sm_hal_timer_impl_t *parent = (sm_hal_timer_impl_t*)malloc(sizeof(sm_hal_timer_impl_t));
    if(parent == NULL) return NULL;

    parent->name = _name;
    parent->arg = _arg;
    parent->base.funtion = _funtion;
    parent->base.proc = _proc;
    parent->callback = timer_callback;
    parent->base.handle = _handle;
    return &parent->base;
}   

sm_hal_timer_t* sm_hal_timer_init_static(sm_hal_timer_proc_t *_proc, timerfuntion_t _funtion, const char *_name, void *_arg,
        void *_handle,sm_hal_timer_static_t *buff){
    sm_hal_timer_impl_t *parent = (sm_hal_timer_impl_t*)buff;
    if(parent == NULL) return NULL;

    parent->name = _name;
    parent->arg = _arg;
    parent->base.funtion = _funtion;
    parent->base.proc = _proc;
    parent->callback = timer_callback;
    parent->base.handle = _handle;
    return &parent->base;
}

void sm_hal_timer_deinit(sm_hal_timer_t* _this){
    sm_hal_timer_impl_t *parent = (sm_hal_timer_impl_t*)_this;
    if(_this->proc->close != NULL){
        _this->proc->close(_this);
    }
    free(parent);
}

static void timer_callback(sm_hal_timer_t *_this){
    sm_hal_timer_impl_t *parent = (sm_hal_timer_impl_t*)_this;
    if(_this->funtion != NULL)
        _this->funtion(parent->arg);
};
void sm_hal_timer_set_callback(sm_hal_timer_t *_this,timerfuntion_t _funtion,void* _arg){
    sm_hal_timer_impl_t *parent = (sm_hal_timer_impl_t*)_this;

    _this->funtion = _funtion;
    parent->arg = _arg;
}

void sm_hal_timer_callback(sm_hal_timer_t *_this){
    sm_hal_timer_impl_t *parent = (sm_hal_timer_impl_t*)_this;
    if(parent->callback != NULL)
        parent->callback(_this);
} // Calling funtion in timer interrupt


