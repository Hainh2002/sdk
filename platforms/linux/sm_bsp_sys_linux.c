//
// Created by vnbk on 27/02/2024.
//
#include <stdio.h>
#include <time.h>

#include "sm_bsp_linux_porting.h"

void logger_put(const char* log){
    printf("%s\n", log);
}

int32_t sm_bsp_init(){
    return 0;
}