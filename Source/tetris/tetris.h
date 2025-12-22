#ifndef _TETRIS_H_
#define _TETRIS_H_

#include <stdint.h> // Serve per capire tipi come uint8_t se usati

/* ==============================================
   DEFINIZIONE DELLE COSTANTI PUBBLICHE
   ============================================== */
#define ROWS 20      
#define COLS 10      
#define BLOCK_SIZE 15 

// Colori utili
#define Black 0x0000
#define White 0xFFFF
#define Red   0xF800
#define Blue  0x001F
#define Green 0x07E0

/* ==============================================
   PROTOTIPI DELLE FUNZIONI (Il "Menu")
   ============================================== */

// Chiama questa funzione nel main per preparare la grafica
void Init_Game_Graphics(void);
// Chiama questa per resettare la logica (matrice vuota)
void Reset_Board(void);
void Draw_Block(int r, int c, uint16_t color);

// Aggiungeremo qui altre funzioni man mano (es. Draw_Piece, Game_Step...)

#endif