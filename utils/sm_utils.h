/*
 * sm_math.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Admin
 */

#ifndef UTILS_INCLUDE_SM_MATH_H_
#define UTILS_INCLUDE_SM_MATH_H_

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief
 * @param max
 * @param min
 * @return
 */
static inline int sm_rand(int max, int min){
    return (rand() % (max - min + 1) + min);
}

static inline int32_t sm_is_in_bound(int32_t _value, int32_t _min, int32_t _max, int32_t _default){
    return (_value >= _min) && (_value <= _max) ? _value : _default;
}


#endif /* UTILS_INCLUDE_SM_MATH_H_ */
