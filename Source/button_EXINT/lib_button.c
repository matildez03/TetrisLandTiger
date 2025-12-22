
#include "button.h"
#include "LPC17xx.h"

/**
 * @brief  Function that initializes Buttons
 */
void BUTTON_init(void) {

  // P2.10 -> EINT0 (01 su bits 20-21)
  LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(3<<20)) | (1<<20);
  LPC_GPIO2->FIODIR  &= ~(1<<10);

  // P2.11 -> EINT1 (01 su bits 22-23)
  LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(3<<22)) | (1<<22);
  LPC_GPIO2->FIODIR  &= ~(1<<11);

  // P2.12 -> EINT2 (01 su bits 24-25)
  LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(3<<24)) | (1<<24);
  LPC_GPIO2->FIODIR  &= ~(1<<12);

  // Pull-up su P2.10, P2.11, P2.12 (00 su PINMODE4)
  LPC_PINCON->PINMODE4 &= ~((3<<20) | (3<<22) | (3<<24));

  // Edge sensitive
  LPC_SC->EXTMODE = 0x7;

  // (opzionale ma consigliato) falling edge, perché i tasti sono active-low
  LPC_SC->EXTPOLAR &= ~0x7;

  NVIC_EnableIRQ(EINT0_IRQn);
  NVIC_EnableIRQ(EINT1_IRQn);
  NVIC_EnableIRQ(EINT2_IRQn);
}

