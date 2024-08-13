/*
 * sm_dft.h
 *
 *  Created on: Sep 7, 2023
 *      Author: Admin
 *
 *      Discrete Fourier Transform Algorithm
 *
 *
 */

#ifndef SM_DFT_H_
#define SM_DFT_H_

#include<stdio.h>
#include<math.h>
#define PI 3.14159265

typedef struct sm_dft_val sm_dft_val_t;

struct sm_dft_val{
    double *real;
    double *img;
    double *magnitude;
};
/**
 *
 * @param dft
 * @param func
 * @param len
 */
void sm_calculateDFT(sm_dft_val_t *dft,double *func/*Signal Value*/,size_t len /*Number Sample*/);
/**
 *
 * @param dft
 * @param len
 * @param Fs
 * @return
 */
double sm_calculateFreq(sm_dft_val_t *dft,unsigned int len/*Number Sample*/,double Fs /*Frequency Sample Rate */);


#endif /* SM_DFT_H_ */
