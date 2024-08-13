/*
 * sm_IIR.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Admin
 */

#ifndef UTILS_INCLUDE_SM_IIR_H_
#define UTILS_INCLUDE_SM_IIR_H_

/*
*
* IIR (Infinite Impulse Response) First-Order Filter
*
* Implements a discrete-time first-order IIR filter.
* Constant term alpha sets the amount of filtering. 0 = no filtering, 1 = max. filtering
*
* Last changed: 01 Dec 2019
*
*/
typedef struct {
	float alpha;
	float out;
} sm_IIR_t;
/**
 * @brief
 * @param filt
 * @param alpha
 */
static inline void sm_IIR_init(sm_IIR_t *filt, float alpha) {
	filt->alpha = alpha;
	filt->out   = 0.0f;
}
/**
 * @brief
 * @param filt
 * @param in
 * @return
 */
static inline float sm_IIR_update(sm_IIR_t *filt, float in) {
	filt->out = filt->alpha * filt->out + (1.0f - filt->alpha) * in;
	return filt->out;
}

#endif /* UTILS_INCLUDE_SM_IIR_H_ */
