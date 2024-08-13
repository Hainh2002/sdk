/*
 * sm_dft.c
 *
 *  Created on: Sep 7, 2023
 *      Author: Admin
 */


#include "sm_dft.h"
#include "stdbool.h"

void sm_calculateDFT(sm_dft_val_t *dft,double *func,size_t len){
	double a = 0;
	bool inverse = false;
	for(unsigned int i = 0;i<len;i++){
		dft->real[i] = 0;
		dft->img[i] = 0;
		for(unsigned int j = 0;j<len;j++){

			a = (-2) * PI * i * j / ((float) len);
			if (inverse)
				a = -a;

			double ca = cos(a);
			double sa = sin(a);

			dft->real[i] += func[j]*ca;
			dft->img[i] += func[j]*sa;
		}
		if (!inverse) {
			dft->real[i] /= len;
			dft->img[i] /= len;
		}
	}
}

double sm_calculateFreq(sm_dft_val_t *dft,unsigned int len,double Fs){

	double max = 0;
	unsigned int max_index = 0;
	for(unsigned int i = 0;i<len;i++){
		dft->magnitude[i] = sqrt(dft->real[i]*dft->real[i] + dft->img[i]*dft->img[i]);
//		printf("%f\n",dft->magnitude[i]);
		if(max < dft->magnitude[i]){
			max = dft->magnitude[i];
			max_index = i;
		}
	}

//	printf("max index %d \n",max_index);
//	printf("max : %f\n",max);

	double freq = max_index * Fs/len;

	return freq;
}
