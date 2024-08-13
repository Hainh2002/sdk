//
// Created by vnbk on 22/03/2023.
//
#include <stdlib.h>
#include <memory.h>

#include "sm_segment.h"
#include "sm_crc.h"

static sm_segment_t g_segment_default = {
        .m_size = 0,
        .m_index = 0,
        .m_addr = 0,
        .m_length = 0,
        .m_crc = 0,
        .m_is_last = 0,
};

sm_segment_t *sm_seg_create(){
    sm_segment_t* obj = malloc(sizeof(sm_segment_t));
    if(!obj)
        return NULL;
    memset(obj, 0, sizeof(sm_segment_t));
    return obj;
}
sm_segment_t *sm_seg_create_default(){
    memset(g_segment_default.m_data, 0, sizeof(g_segment_default.m_data));

    return &g_segment_default;
}
int32_t sm_seg_destroy(sm_segment_t* self){
    if(!self){
        return -1;
    }
    free(self);
    return 0;
}

int32_t sm_seg_set_info(sm_segment_t* _self, int32_t index, int32_t addr, int32_t size, uint16_t crc){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return -1;
    }
    self->m_index = index;
    self->m_addr = addr;
    self->m_crc = crc;
    self->m_size = size;
    self->m_length = 0;
}
int32_t sm_seg_fill_data(sm_segment_t* _self, const uint8_t* data, int32_t len){
    sm_segment_t* self = (sm_segment_t*)_self;
    int32_t index = 0;
    if(!self){
        return -1;
    }
    for(index = 0; index < len; index++){
        self->m_data[self->m_length++] = data[index];
        if(self->m_length > self->m_size)
            return (len - index);
    }
    return len;
}
int32_t sm_seg_set_addr(sm_segment_t* _self, int32_t addr){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return -1;
    }
    self->m_addr = addr;
    return 0;
}
int32_t sm_seg_set_size(sm_segment_t* _self, int32_t size){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return -1;
    }
    self->m_size = size;
    return size;
}
int32_t sm_seg_set_crc(sm_segment_t* _self, uint16_t crc){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return -1;
    }
    self->m_crc = crc;
    return 0;
}

int32_t sm_seg_set_last_segment(sm_segment_t* _self, uint8_t is_last){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return -1;
    }
    self->m_is_last = is_last;
    return 0;
}

int32_t sm_seg_is_valid(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return -1;
    }
    return self->m_crc == sm_CRC_CalculateCRC16(self->m_data, self->m_length);
}

int32_t sm_seg_is_full(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    return self ? (self->m_length == self->m_size) : -1;
}

int32_t sm_seg_get_index(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    return self ? self->m_index : -1;
}
int32_t sm_seg_get_addr(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    return self ? self->m_addr : -1;
}
int32_t sm_seg_get_crc(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    return self ? self->m_crc : -1;
}
int32_t sm_seg_get_length(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    return self ? self->m_length : -1;
}

int32_t sm_seg_get_last_segment(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    return self ? self->m_is_last : -1;
}

uint8_t* sm_seg_get_data(sm_segment_t* _self){
    sm_segment_t* self = (sm_segment_t*)_self;
    if(!self){
        return NULL;
    }
    return self->m_data;
}