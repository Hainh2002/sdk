#ifndef SM_ERROR_H_
#define SM_ERROR_H_

#include "stdint.h"

typedef enum {
    // ERROR CODE
    SM_ERR_UNKNOWN          = (int16_t)-99,
    SM_ERR_INVALID_PARAM    = -1,
    // SUCC CODE
    SM_ERR_OK               = 0,
    // WARNING CODE
    SM_WRN_BUSY             = 1,
}ERROR_CODE;

#endif