//
// Created by vnbk on 28/03/2023.
//
#include <stdio.h>
#ifdef __linux
#include <unistd.h>
#endif

#include "sm_boot_slave.h"
#include "sm_fw.h"
#include "sm_crc.h"
#include "sm_string_t.h"
#include "sm_intel_hex.h"

#define TAG "sm_file_boot_input"

static sm_fw_signature_t g_new_fw_signature = {
        .m_size = 0,
        .m_crc = 0,
        .m_addr = 0,
        .m_is_valid = 0,
        .m_version = {0x01, 0x00, 0x02},
        .m_data_address_offset = -1
};

static string_t* g_file_path = NULL;
static int32_t g_total_seg = 0;

static char* g_file_buffer = NULL;

int32_t sm_file_init(){
    FILE* hex_file = fopen(g_file_path->buffer, "rb");
    if(!hex_file){
        printf("Could NOT open the HEX file %s\n", g_file_path->buffer);
        return -1;
    }

    string_t* bin_file_name = string_createz(g_file_path->buffer);
    bin_file_name = string_appendz(bin_file_name, ".bin");

    FILE* bin_file = fopen(bin_file_name->buffer, "w");
    if(!bin_file){
        printf("Could NOT open the BIN file\n");
        return -1;
    }

    char* start_line = NULL;
    uint8_t buff_hex[50]; /*seg_data[1024];*/
    size_t size_line = 0;
    size_t line_len = 0;
    int line_number = 0;
    bool bypass = false;

    while (line_len >= 0){
        line_len = getline(&start_line, &size_line, hex_file);

        line_number++;

        if(!strcmp(&start_line[line_len-2], "\r\n")){
            line_len -= 2;
        }else if(start_line[line_len-1] == '\n'){
            line_len--;
        }

        int hex_length = string_to_hex(start_line, line_len, buff_hex);
        intel_hex *p_record = intel_hex_process(buff_hex, hex_length);

        if(p_record == NULL){
            printf("recode is NOT hex file format\n");
            return -1;
        }

        if(p_record->record_type == INTEL_HEX_Extended_Linear_Address){
            uint32_t addr_16_31 = (uint32_t)(((uint32_t)p_record->data[0]<<8) | (uint32_t)p_record->data[1]);
            uint32_t addr_0_15 = ((uint32_t)p_record->addr[0] << 8) | (uint32_t)p_record->addr[1];

//            bypass = true;

            if(!g_new_fw_signature.m_addr){
                g_new_fw_signature.m_addr = (addr_16_31<<16)  + addr_0_15;
            }

            printf("Recode Type = 4, line_number: %d\n", line_number);
        }else if(p_record->record_type == INTEL_HEX_Extended_Segment_Address){
            uint32_t addr_4_19 = ((uint32_t)p_record->data[0]<<8) | (uint32_t)p_record->data[1];
            uint32_t addr_0 = ((uint32_t)p_record->addr[0] << 8) | (uint32_t)p_record->addr[1];

            if(!g_new_fw_signature.m_addr){
                g_new_fw_signature.m_addr = (addr_4_19<<4) + addr_0;
            }

            printf("Recode Type = 2, line_number: %d\n", line_number);
        }else if(p_record->record_type == INTEL_HEX_Data){
            if(bypass) {
                continue;
            }

            if(g_new_fw_signature.m_data_address_offset < 0){
                g_new_fw_signature.m_data_address_offset = (p_record->addr[0]<<8) + p_record->addr[1];
            }

            fwrite(p_record->data, 1, p_record->byte_count, bin_file);
        }else if(p_record->record_type == INTEL_HEX_EndFile){
            printf("Finished read hex file\n");

            break;
        }
    }
    fclose(hex_file);
    fclose(bin_file);
//    printf("Total line is read: %d\n", line_number);
//    printf("Total CRC after calculate segments: 0x%2X\n", total_crc);

    bin_file = fopen(bin_file_name->buffer, "r");
    if(!bin_file){
        string_dispose(bin_file_name);
        return -1;
    }

    fseek(bin_file, 0L, SEEK_END);
    int file_size = (int)ftell(bin_file);

    fseek(bin_file, 0L, SEEK_SET);

    g_file_buffer = malloc(file_size * sizeof(char));

    fread(g_file_buffer, sizeof(char), file_size, bin_file);

    /// Close file
    fclose(bin_file);
    string_dispose(bin_file_name);

    uint16_t crc = sm_CRC_CalculateCRC16((uint8_t*)g_file_buffer, file_size);

    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_size = file_size;
    g_new_fw_signature.m_addr += g_new_fw_signature.m_data_address_offset;
    g_new_fw_signature.m_crc = crc;

    g_total_seg = file_size/1024;
    if(file_size%1024 != 0){
        g_total_seg += 1;
    }

    printf("Firmware address: 0x%x\n", g_new_fw_signature.m_addr);
    printf("Total size of new fw is : %d and is dived with total seg: %d\n", g_new_fw_signature.m_size, g_total_seg);

    return 0;
}

int32_t sm_file_free(){
    if(g_file_buffer){
        free(g_file_buffer);
        g_file_buffer = NULL;
    }
    g_new_fw_signature.m_size = 0;
    g_new_fw_signature.m_addr = 0;
    g_new_fw_signature.m_crc = 0;
    g_new_fw_signature.m_is_valid = 0;
    g_new_fw_signature.m_data_address_offset = -1;

    return 0;
}

int32_t sm_file_get_fw_info(sm_fw_signature_t* _fw){
    sm_fw_signature_clone(_fw, &g_new_fw_signature);
    return 1;
}

int32_t sm_file_get_total_seg(){
    return g_total_seg;
}

#define FIRMWARE_ADDR_START  0x10000

//static int32_t g_count = 0;

int32_t sm_file_get_seg_fw(sm_segment_t* _seg){
    if(g_file_buffer){
       /* if(g_count < 20){
            usleep(100*1000);
            g_count++;
            return 0;
        }*/
        int len = 1024;
        uint16_t crc = 0;
        int seg_index = _seg->m_index;
        if(seg_index >= (g_total_seg-1)){
            len = g_new_fw_signature.m_size - (seg_index)*1024;
            sm_seg_set_last_segment(_seg, 1);
            printf("Last segment is load from storage, size of last segment: %d\n", len);
        }else{
            len = 1024;
            sm_seg_set_last_segment(_seg, 0);
        }
        crc = sm_CRC_CalculateCRC16((uint8_t*)g_file_buffer + seg_index*1024, len);

        sm_seg_set_info(_seg, seg_index, g_new_fw_signature.m_addr + seg_index*1024, len, crc);

        sm_seg_fill_data(_seg, (uint8_t*)(g_file_buffer + seg_index*1024), len);

//        g_count = 0;

        return 1;
    }
    return -1;
}

sm_boot_input_if_t g_boot_input_proc = {
        .init = sm_file_init,
        .free = sm_file_free,
        .get_total_seg = sm_file_get_total_seg,
        .get_fw_info = sm_file_get_fw_info,
        .get_seg_fw = sm_file_get_seg_fw,
        .process = NULL
};

sm_boot_input_if_t* sm_get_file_boot_input(const char* file_path){
    if(file_path){
        string_dispose(g_file_path);
        g_file_path = string_createz(file_path);
        return &g_boot_input_proc;
    }
    return NULL;
}