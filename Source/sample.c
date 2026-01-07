#include "LPC17xx.h"
#include "GLCD/GLCD.h"
#include "TouchPanel/TouchPanel.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "button_EXINT/button.h"
#include "joystick/joystick.h"
#include "led/led.h"
#include "tetris/tetris.h"


#define SIMULATOR 1

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

#define tim0Period  0x0007A120 // FLAG DA MODIFICARE
#define RITPeriod  0x0007A120 // FLAG DA MODIFICARE

int main(void)
{
	SystemInit();  												/* System Initialization (i.e., PLL)  */
	BUTTON_init();												/* Inizializzazione Buttons 					*/
	LCD_Initialization();									/* Inizializzazione Display 					*/
	joystick_init();											/* Inizializzazione Joystick 					*/
	LED_init();
 Init_Game_Graphics();         // Disegna la griglia
	
	init_RIT(RITPeriod);									/* RIT Initialization a 20msc , 30msc = 750000 / 50 msec = 0x004C4B40   	*/
	enable_RIT();

	init_timer(0, 0, 0, 3, tim0Period); 	// Timer0 inizializzazione con periodo 0,02 sec per gravitystep //1s*25Mhz = 25.000.000 = 0x017D7840
	enable_timer(0);

	//POWER DOWN MODE
	LPC_SC->PCON |= 0x1;      // set PM0 = 1
	LPC_SC->PCON &= ~(0x2);  // set PM1 = 0
	
	static uint8_t last_soft = 0;
	
	//wait for interrupt
  while (1)	
  {
	
		if (key1_event){
			key1_event = 0;
			toggle_pause();
		}
		
		if(left_activate == 1){
			tetris_moveLeft();
			left_activate = 0;
		}
		
		if(right_activate == 1){
			tetris_moveRight();
			right_activate = 0;
		}
		
		if(up_activate == 1){
			tetris_rotate();
			up_activate = 0;
		}
		
		if (gravity_event == 1) {
   tetris_gravityStep();
			gravity_event = 0;
		}

		if (softdrop_on != last_soft) {
    last_soft = softdrop_on;
		
    LPC_TIM0->TCR = 0;  // stop
    LPC_TIM0->TC  = 0;  // reset counter 
    LPC_TIM0->TCR = 1;  // start
			
			

    LPC_TIM0->MR0 = softdrop_on ? (tim0Period/2) : tim0Period; // 0.005s / 0.02s
		}

		if(score_dirty){
			score_dirty = 0;
			update_score();
		}
		
		if (key2_event) {
			key2_event = 0;
			tetris_hardDrop();
		}
		
		__ASM("wfi");
  }
}



/*
END OF FILE
*********************************************************************************************************/