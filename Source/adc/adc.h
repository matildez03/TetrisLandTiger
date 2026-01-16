#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>
#warning "INCLUDING Source/adc/adc.h"


/* lib_adc.c */
void ADC_init (void);
void ADC_start_conversion (void);

/* IRQ_adc.c */
void ADC_IRQHandler(void);

/* Valore ADC corrente (0..0xFFF) */
extern volatile uint16_t AD_current;
uint32_t compute_mr0_from_adc(uint16_t adc, uint8_t softdrop);


#endif
