#ifndef TETRIS_GFX_H
#define TETRIS_GFX_H

#include <stdint.h>
#include "tetris.h"   // per ROWS/COLS e per leggere board/gameState ecc.

#define BLOCK_SIZE 15


void Init_Game_Graphics(void);
void Draw_Block(int r, int c, uint16_t color);
void Draw_Grid(void);        
void redraw_board(void);
void update_score(void);

/* utili “grafici” usati dalla logica (wrapper) */
void gfx_draw_piece_at(int r0, int c0, int id, int rot, uint16_t color);
void gfx_erase_piece_at(int r0, int c0, int id, int rot);

#endif
