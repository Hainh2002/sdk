//
// Created by vnbk on 22/03/2023.
//

#ifndef SM_FW_H
#define SM_FW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <memory.h>
#include "sm_segment.h"

#define SM_FW_VERSION_LENGTH    3

typedef struct sm_fw sm_fw_t;

typedef struct sm_fw_signature{
    uint8_t m_version[SM_FW_VERSION_LENGTH];
    uint32_t m_addr;
    uint32_t m_size;
    uint16_t m_crc;
    int32_t m_data_address_offset;
    uint8_t m_is_valid;
}sm_fw_signature_t;

static inline int32_t sm_fw_signature_clone(sm_fw_signature_t* _self, const sm_fw_signature_t* _other){
    if(!_other){
        return -1;
    }
    memcpy(_self->m_version, _other->m_version, SM_FW_VERSION_LENGTH);
    _self->m_addr = _other->m_addr;
    _self->m_size = _other->m_size;
    _self->m_crc = _other->m_crc;
    _self->m_is_valid = _other->m_is_valid;
    return 0;
}

static inline int32_t sm_fw_signature_reset(sm_fw_signature_t* _this){
    if(!_this){
        return -1;
    }
    memset(_this->m_version, 0, SM_FW_VERSION_LENGTH);
    _this->m_size = 0;
    _this->m_addr = 0;
    _this->m_crc = 0;
    _this->m_is_valid = 0;
    return 0;
}

typedef struct sm_fw_storage_opt sm_fw_storage_opt_t;

typedef struct {
    int32_t (*init)(sm_fw_storage_opt_t*);
    int32_t (*free)(sm_fw_storage_opt_t*);
    int32_t (*get_info)(sm_fw_storage_opt_t*, void*);
    int32_t (*get_signature)(sm_fw_storage_opt_t*, sm_fw_signature_t*);
    int32_t (*set_signature)(sm_fw_storage_opt_t*, const sm_fw_signature_t*);
    int32_t (*store_signature)(sm_fw_storage_opt_t*);
    int32_t (*read_all)(sm_fw_storage_opt_t*, uint8_t*);
    int32_t (*read_segment)(sm_fw_storage_opt_t*, uint32_t, uint32_t);
    int32_t (*write_segment)(sm_fw_storage_opt_t*, uint32_t, const uint8_t*, uint32_t);
    int32_t (*write_all)(sm_fw_storage_opt_t*, const uint8_t*, uint32_t);
    int32_t (*erase_segment)(sm_fw_storage_opt_t*, uint32_t);
    int32_t (*erase_full)(sm_fw_storage_opt_t*);
}sm_fw_storage_opt_proc_t;

struct sm_fw_storage_opt{
    const sm_fw_storage_opt_proc_t* m_proc;
};

sm_fw_t* sm_fw_create(const sm_fw_storage_opt_t* _mem_opt);
sm_fw_t* sm_fw_create_default(const sm_fw_storage_opt_t* _mem_opt);
int32_t sm_fw_destroy(sm_fw_t* _self);

int32_t sm_fw_set_signature(sm_fw_t* _self, const sm_fw_signature_t* _sign);
const sm_fw_signature_t* sm_fw_get_signature(sm_fw_t* _self);

int32_t sm_fw_is_valid(sm_fw_t* _self);
int32_t sm_fw_format(sm_fw_t* _self);
int32_t sm_fw_write_segment(sm_fw_t* _self, const sm_segment_t* _segment);
int32_t sm_fw_clone(sm_fw_t* _self, sm_fw_t* _other);

#ifdef __cplusplus
};
#endif

#endif //SM_FW_H
