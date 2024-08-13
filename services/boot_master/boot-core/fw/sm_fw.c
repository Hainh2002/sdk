//
// Created by vnbk on 22/03/2023.
//
#include "sm_fw.h"
#include "sm_crc.h"

#define TAG "sm_fw"

struct sm_fw{
    sm_fw_storage_opt_t* m_mem_opt;
    sm_fw_signature_t m_signature;
};

static sm_fw_t g_fw_default = {
        .m_signature = {
                .m_addr = 0,
                .m_is_valid = 0,
                .m_crc = 0,
                .m_size = 0,
                .m_version = {0,}
        },
        .m_mem_opt = NULL
};

sm_fw_t* sm_fw_create(const sm_fw_storage_opt_t* _mem_opt){
    sm_fw_t* obj = NULL;
    if(!_mem_opt){
        return NULL;
    }
    obj = (sm_fw_t*)malloc(sizeof(sm_fw_t));
    if(!obj){
        return NULL;
    }
    obj->m_mem_opt = _mem_opt;
    obj->m_signature.m_size = 0;
    obj->m_signature.m_crc = 0;
    obj->m_signature.m_is_valid = 0;
    obj->m_signature.m_addr = 0;

    obj->m_mem_opt->m_proc->init(obj->m_mem_opt);

    return obj;
}
sm_fw_t* sm_fw_create_default(const sm_fw_storage_opt_t* _mem_opt){
    if(!_mem_opt){
        return NULL;
    }
    g_fw_default.m_mem_opt = _mem_opt;
    g_fw_default.m_signature.m_size = 0;
    g_fw_default.m_signature.m_crc = 0;
    g_fw_default.m_signature.m_is_valid = 0;
    g_fw_default.m_signature.m_addr = 0;

    g_fw_default.m_mem_opt->m_proc->init( g_fw_default.m_mem_opt);

    return &g_fw_default;
}
int32_t sm_fw_destroy(sm_fw_t* _self){
    if(!_self){
        return -1;
    }
    _self->m_mem_opt->m_proc->free(_self->m_mem_opt);
    free(_self);
    return 0;
}

int32_t sm_fw_set_signature(sm_fw_t* _self, const sm_fw_signature_t* _sign){
    if(!_self){
        return -1;
    }
    sm_fw_signature_clone(&_self->m_signature, _sign);
    return 0;
}

const sm_fw_signature_t* sm_fw_get_signature(sm_fw_t* _self){
    if(!_self){
        return NULL;
    }
    return &_self->m_signature;
}

int32_t sm_fw_is_valid(sm_fw_t* _self){
    if(!_self){
        return -1;
    }
    /*uint16_t crc_val = CRC_CalculateCRC16(_self->m_mem_opt->m_proc->read_all(_self->m_mem_opt), _self->m_signature.m_size);
    return (crc_val == _self->m_signature.m_crc);*/
}

int32_t sm_fw_format(sm_fw_t* _self){
    if(!_self){
        return -1;
    }
    return _self->m_mem_opt->m_proc->erase_full(_self->m_mem_opt);
}

int32_t sm_fw_write_segment(sm_fw_t* _self, const sm_segment_t* _segment){
    int32_t size;
    uint32_t addr;
    const uint8_t* code;
    if(!_self){
        return -1;
    }
    addr = sm_seg_get_addr((sm_segment_t*)_segment);
    size = sm_seg_get_length((sm_segment_t*)_segment);
    code = sm_seg_get_data((sm_segment_t*)_segment);

   /* if(_segment->m_length < _segment->m_size){
        LOG_INF(TAG, "Last segment is write");
    }*/

    return _self->m_mem_opt->m_proc->write_segment(_self->m_mem_opt, addr, code, size);
}

int32_t sm_fw_clone(sm_fw_t* _self, sm_fw_t* _other){
    if(!_self){
        return -1;
    }
   /* for(uint32_t i = 0; i < FLASH_IMAGE_TOTAL_SIZE; i += SEGMENT_MEMORY_SIZE ){
        memcpy(p_fw->segment->data, write_buff+i, SEGMENT_MEMORY_SIZE);
        rc = p_fw->write_code_flash(p_fw->signature.addr + i, p_fw->segment->data, SEGMENT_MEMORY_SIZE);
        if( rc < 0 ) break;
    }*/
    return 0;
}
