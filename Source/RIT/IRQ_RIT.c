#include "LPC17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "../timer/timer.h"
#include "../GLCD/GLCD.h"
#include "../tetris/tetris.h"

volatile uint8_t right_activate;
volatile uint8_t down_activate;
volatile uint8_t left_activate;
volatile uint8_t up_activate;

volatile int down_0 = 0;
volatile int down_1 = 0;
volatile int down_2 = 0;
extern char led_value;
extern volatile uint8_t key2_event;
extern volatile uint8_t key1_event;

void RIT_IRQHandler (void)
{					
	static int J_select = 0;
	static int J_down = 0;
	static int J_left = 0;
	static int J_right = 0;
	static int J_up = 0;
	
	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0){	
		/* Joytick J_Select pressed p1.25*/
		
		J_select++;
		switch(J_select){
			case 1:
 				// non fa nulla
				break;
			default:
				break;
		}
	}
	else{
			J_select=0;
	}
	
	if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	
		/* Joytick J_Down pressed p1.26 --> using J_DOWN due to emulator issues*/
		J_down++;
		// Qui non usiamo "J_down == 1" perché vogliamo che scenda veloce 
    // finché teniamo premuto il joystick
		if(gameState == GAME_RUNNING){
			if(J_down == 1){
			tetris_gravityStep();
			}
			if(J_down % 2 == 0) { // Regola la velocità del drop (ogni 2 cicli RIT)
				 softdrop_on = 1;
			}
    }
	}
	else{
		J_down=0;
		down_activate =0;
		softdrop_on = 0;
	}
	
	if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	
		/* Joytick J_Left pressed p1.27*/
		J_left++;
		switch(J_left){
			case 1:
				left_activate =1;
			if(gameState == GAME_RUNNING) tetris_moveLeft();
			
				break;
			default:
				break;
		}
	}
	else{
			J_left=0;
			left_activate =0;
	}
	
	if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	
		/* Joytick J_right pressed p1.28*/
		
		J_right++;
		switch(J_right){
			case 1:
			right_activate =1;
			if(gameState == GAME_RUNNING) tetris_moveRight();
			
				break;
			default:
				break;
		}
	}
	else{
			J_right=0;
			right_activate =0;
	}
	
	if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	
		/* Joytick J_up pressed p1.29*/
		
		J_up++;
		switch(J_up){
			case 1:
				up_activate =1;
				if(gameState == GAME_RUNNING) tetris_rotate(); 
	
				break;
			default:
				break;
		}
	}
	else{
			J_up=0;
			up_activate = 0;
	}
	
	
	/* button management */
	if(down_0 != 0){
		down_0++; 
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){	/* INT0 pressed */			
			switch(down_0){
				case 2:				
				
					break;
				default:
					break;
			}
		}
		else {	/* button released */
			down_0=0;			
			NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
		}
		LPC_RIT->RICTRL |= 0x1;
	}
	
	//key1
	// KEY1 debounce + wait release (P2.11 active low)
	
	if (down_1 != 0) {  // debounce press
		down_1++;
		if(down_1 == 2){
			key1_event = 1;
		}
		
    if ((LPC_GPIO2->FIOPIN & (1<<11)) != 0) {   // released
      down_1 = 0;
      NVIC_EnableIRQ(EINT1_IRQn);
      LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(3 << 22)) | (1 << 22);
    }
		LPC_RIT->RICTRL |= 0x1;
	}

	
	//key2: hard drop
	if(down_2 != 0){
		down_2++;
		if(down_2 == 2){
			key2_event = 1;
		}	
		if((LPC_GPIO2->FIOPIN & (1<<12)) != 0){	/* KEY2 NOT pressed */			
			down_2=0;			
			NVIC_EnableIRQ(EINT2_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(3 << 24)) | (1 << 24);		
		}	
		LPC_RIT->RICTRL |= 0x1;
	}
	
		reset_RIT();
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
  return;
}

/******************************************************************************
**                            End Of File
******************************************************************************/
