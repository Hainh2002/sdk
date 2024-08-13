#include <sm_hal_can.h>


typedef struct sm_hal_can_impl_t
{
    /* data */
    sm_hal_can_t base;
    sm_fifo_handle_t buff;
    uint16_t buff_size;
    bool tx_complete;

    void (*rx_callback)(sm_hal_can_t *_this,sm_hal_can_mgs_t *mgs);
    void (*tx_callback)(sm_hal_can_t *_this);

}sm_hal_can_impl_t;
sm_hal_can_impl_t *this;
sm_hal_can_mgs_t can_mgs_array[10];

static int32_t can_read(sm_hal_can_t *_this,sm_hal_can_mgs_t *mgs);
static void can_rx_callback(sm_hal_can_t *_this,sm_hal_can_mgs_t *mgs);
static void can_tx_callback(sm_hal_can_t *_this);

sm_hal_can_t* sm_hal_can_init(sm_hal_can_proc_t *_proc,void* _handle){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*) malloc (sizeof(sm_hal_can_impl_t));
    if (parent == NULL)
        return NULL;
    parent->buff_size = 10;
    parent->base.proc = _proc;
    parent->rx_callback = can_rx_callback;
    parent->tx_callback = can_tx_callback;
    parent->base.handle = _handle;
    this = parent;
    return &parent->base;
}

void sm_hal_can_deinit(sm_hal_can_t *_this){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*) malloc(sizeof(sm_hal_can_t));
    if(_this->proc->close != NULL)
        _this->proc->close(_this);
    fifo_deinit (&parent->buff);
    free(parent);
}

void sm_hal_can_deinit(sm_hal_can_t *_this);

static int32_t can_read(sm_hal_can_t *_this,sm_hal_can_mgs_t *mgs){

    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*)_this;
    if (fifo_pop (&parent->buff, (void*) mgs) == true)
    {
        return 1;
    }
    return 0;
}


static void can_rx_callback(sm_hal_can_t *_this,sm_hal_can_mgs_t *_mgs){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*)_this;
}

static void can_tx_callback(sm_hal_can_t *_this){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*)_this;
    parent->tx_complete = true;
}

void sm_hal_can_rx_callback(sm_hal_can_t *_this,sm_hal_can_mgs_t *_mgs){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*)_this;
    parent->rx_callback(_this,_mgs);
}

void sm_hal_can_tx_callback(sm_hal_can_t *_this){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*)_this;
    parent->tx_callback(_this);
}

int32_t sm_hal_can_write(sm_hal_can_t *_this,sm_hal_can_mgs_t *_mgs){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*) _this;
    parent->tx_complete = false;
    _this->proc->write (_this, _mgs);
    return 0;
}
int32_t sm_hal_can_read(sm_hal_can_t *_this,sm_hal_can_mgs_t *_mgs){
   return can_read(_this,_mgs);
}
int32_t sm_hal_can_set_baudrate(sm_hal_can_t *_this,uint32_t _baudrate){
    return _this->proc->set_baudrate(_this,_baudrate);
}
int32_t sm_hal_can_open(sm_hal_can_t *_this){
    return _this->proc->open(_this);
}
int32_t sm_hal_can_close(sm_hal_can_t *_this){
    return _this->proc->close(_this);
}
void sm_hal_can_set_rx_callback(sm_hal_can_t *_this, func_can_rx_callback callback){
    sm_hal_can_impl_t *parent = (sm_hal_can_impl_t*) _this;
    parent->rx_callback = callback;
}
