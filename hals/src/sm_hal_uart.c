#include <sm_hal_delay.h>
#include <sm_hal_uart.h>



#define MAX_TIMEOUT_TRANF 2000
#define MAX_BUFFER_LENGTH 2048
static void uart_rx_callback(sm_hal_uart_t *_this,uint8_t *data, uint32_t len);
static void uart_tx_callback(sm_hal_uart_t *_this);

typedef struct sm_hal_uart_ipml
{
    /* data */
    sm_hal_uart_t base;
    sm_fifo_handle_t buff;
	volatile bool tx_complete;
    sm_hal_uart_rx_irq rx_callback;
	sm_hal_uart_tx_irq tx_callback;
}sm_hal_uart_ipml_t;

sm_hal_uart_t * sm_hal_uart_init(sm_hal_uart_proc_t *_fun,void* _handle){

    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*) malloc(sizeof(sm_hal_uart_ipml_t));
    if(parent == NULL) return NULL;
    parent->base.handle = _handle;
    parent->base.proc = _fun;

    sm_fifo_init(&parent->buff,128,sizeof(uint8_t));
    parent->rx_callback = uart_rx_callback;
    parent->tx_callback = uart_tx_callback;
    parent->tx_complete = false;
    return &parent->base;
}

sm_hal_uart_t* sm_hal_uart_init_static(sm_hal_uart_proc_t *_func,void *_handle,sm_uart_static_t *buff){
	sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*) buff;

    if(parent == NULL) return NULL;
    parent->base.handle = _handle;
    parent->base.proc = _func;

    sm_fifo_init(&parent->buff,128,sizeof(uint8_t));
    parent->rx_callback = uart_rx_callback;
    parent->tx_callback = uart_tx_callback;
    parent->tx_complete = false;
    return &parent->base;

}

void sm_hal_uart_deinit(sm_hal_uart_t *_this){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    sm_fifo_deinit(&parent->buff);

    if(_this->proc->close != NULL)
        _this->proc->close(_this);
    _this->proc = NULL;
    parent->rx_callback = NULL;
    parent->tx_callback = NULL;
    free(parent);
}

static void uart_rx_callback(sm_hal_uart_t *_this,uint8_t *_data, uint32_t _len){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    for(uint16_t i = 0;i<_len;i++)
        sm_fifo_push_irq (&parent->buff, (void*) &_data[i]);
}

static void uart_tx_callback(sm_hal_uart_t *_this){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    parent->tx_complete = true;
}

void sm_hal_uart_rx_callback(sm_hal_uart_t *_this,uint8_t *_data,uint16_t _len){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    parent->rx_callback(_this,_data,_len);
}

void sm_hal_uart_tx_callback(sm_hal_uart_t *_this){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    parent->tx_callback(_this);
}

int32_t sm_hal_uart_write(sm_hal_uart_t *_this,uint8_t* _buff,uint32_t _len){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;

    parent->tx_complete = false;    
    _this->proc->write(_this,_buff,_len);
    uint32_t timeout = 0;
    while (parent->tx_complete != true)
    {
        /* code */
        timeout++;
        if(timeout > MAX_TIMEOUT_TRANF) break;
        sm_hal_delay_ms(1);
    }
    if(timeout < MAX_TIMEOUT_TRANF) return 0;

    return -1;

}

size_t sm_hal_uart_read(sm_hal_uart_t *_this,uint8_t* _buff,uint32_t _len){
    size_t size = 0;
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    for(size = 0;size<_len;size++){
        if(sm_fifo_pop(&parent->buff,&_buff[size])!= true) break;
    }
    return size;
}

size_t sm_hal_uart_peek(sm_hal_uart_t *_this, uint8_t* _buff, uint32_t _len){
	size_t size = 0;
	sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
	for(size = 0; size < _len; size++){
		if(sm_fifo_peek(&parent->buff, &_buff[size]) != true) break;
	}
	return size;
}

void sm_hal_uart_set_rx_callback(sm_hal_uart_t *_this,sm_hal_uart_rx_irq _irq){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    parent->rx_callback = _irq;
}

void sm_hal_uart_set_tx_callback(sm_hal_uart_t *_this,sm_hal_uart_tx_irq _irq){
    sm_hal_uart_ipml_t *parent = (sm_hal_uart_ipml_t*)_this;
    parent->tx_callback = _irq;
}
