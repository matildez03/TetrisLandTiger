#include "tetris_gfx.h"
#include "tetris_colors.h"
#include "LPC17xx.h"
#include <stdio.h>

// funzioni disponibili in glcd.h
extern void LCD_Clear(uint16_t color);
extern void LCD_DrawLine(int x0, int y0, int x1, int y1, uint16_t color);
extern void GUI_Text(int x, int y, uint8_t *text, uint16_t fg, uint16_t bg);

// funzioni interne di grafica 
static void restore_cell_from_board(int r, int c)
{
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;

    if (board[r][c] == 0) {
        Draw_Block(r, c, BG_COLOR);
    } else {
        int id = board[r][c] - 1;     // 0..6
        Draw_Block(r, c, PIECE_COLORS[id]);
    }
}

// UI
void update_score(void)
{
    char buf[16];

    GUI_Text(160, 80,  (uint8_t *)"        ", BG_COLOR, BG_COLOR);
    GUI_Text(160, 120, (uint8_t *)"        ", BG_COLOR, BG_COLOR);

    sprintf(buf, "%lu", (unsigned long)score);
    GUI_Text(160, 80, (uint8_t *)buf, Red, BG_COLOR);

    sprintf(buf, "%lu", (unsigned long)high_score);
    GUI_Text(160, 120, (uint8_t *)buf, Red, BG_COLOR);
}

void Init_Game_Graphics(void){
	LCD_Clear(BG_COLOR);
	int limit_X = COLS * BLOCK_SIZE;  // 150
  int limit_Y = ROWS * BLOCK_SIZE;  // 300

  // area di gioco
	LCD_DrawLine(0, 0, limit_X, 0, White);
	LCD_DrawLine(0, 0, 0, limit_Y, White);
	LCD_DrawLine(limit_X, 0, limit_X, limit_Y, White);
	LCD_DrawLine(0, limit_Y, limit_X, limit_Y, White);

  // griglia
	Draw_Grid();
	
	// testo 
	GUI_Text(170, 20,  (uint8_t *)"TETRIS",   White, Red);
	GUI_Text(160, 60,  (uint8_t *)"Score:",   White, BG_COLOR);
	GUI_Text(160, 80,  (uint8_t *)"0",        Red,   BG_COLOR);
	GUI_Text(160, 100, (uint8_t *)"Highest:", White, BG_COLOR);
	GUI_Text(160, 120, (uint8_t *)"0",        Red,   BG_COLOR);
	GUI_Text(160, 180, (uint8_t *)" Press ", White, Red);
	GUI_Text(160, 200, (uint8_t *)" 'key1' ", White, Red);
	GUI_Text(160, 220, (uint8_t *)" to start ",   White, Red);
}

void Draw_Grid(void)
{
	int r;
	int c;
	int x;
	int y;

	/* linee verticali */
	for (c = 1; c < COLS; c++)
	{
		x = c * BLOCK_SIZE;
		LCD_DrawLine(x, 0, x, ROWS * BLOCK_SIZE, White);
	}

	/* linee orizzontali */
	for (r = 1; r < ROWS; r++)
	{
		y = r * BLOCK_SIZE;
		LCD_DrawLine(0, y, COLS * BLOCK_SIZE, y, White);
	}
}




/* ---- blocchi e board ---- */
// tetramini
void Draw_Block(int r, int c, uint16_t color) {
	int x0 = c * BLOCK_SIZE;
  int y0 = r * BLOCK_SIZE;
	int i;
	
  for (i = 1; i < BLOCK_SIZE - 1; i++) {
      LCD_DrawLine(x0 + 1, y0 + i, x0 + BLOCK_SIZE - 1, y0 + i, color);
  }
}

void redraw_board(void){
	int r;
	int c;
    // pulizia area campo
	for (r = 0; r < ROWS; r++) {
		for (c = 0; c < COLS; c++) {
			Draw_Block(r, c, BG_COLOR);
    }
  }

    // blocchi fissi
  for (r = 0; r < ROWS; r++) {
		for (c = 0; c < COLS; c++) {
			if (board[r][c] != 0) {
				int id = board[r][c] - 1;
				Draw_Block(r, c, PIECE_COLORS[id]);
			}
		}}
		Draw_Grid();
}

// gestione del pezzo corrente (cancellato e riscritto una riga più in basso)

void gfx_draw_piece_at(int r0, int c0, int id, int rot, uint16_t color){
	int i;
	int j;
    
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (piece_cell(id, rot, i, j)) {
				int r = r0 + i;
				int c = c0 + j;
				if (r >= 0){
					Draw_Block(r, c, color);
				}
			}
		}
	}
}

void gfx_erase_piece_at(int r0, int c0, int id, int rot){
	int i;
	int j;
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (piece_cell(id, rot, i, j)) {
				restore_cell_from_board(r0 + i, c0 + j);
			}
		}
	}
}
