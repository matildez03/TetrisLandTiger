#ifndef _TETRIS_H_
#define _TETRIS_H_

#include <stdint.h>

/* Costanti pubbliche */
#define ROWS 20
#define COLS 10
#define BLOCK_SIZE 15

/* Colori utili */
#define Black 0x0000
#define White 0xFFFF
#define Red   0xF800
#define Blue  0x001F
#define Green 0x07E0

typedef enum {
    GAME_PAUSED = 0,
    GAME_RUNNING,
    GAME_OVER
} GameState;

/* Dichiarazione (non definizione!) */
extern volatile GameState gameState;

/* Prototipi */
void Init_Game_Graphics(void);
void Reset_Board(void);
void Draw_Block(int r, int c, uint16_t color);
void spawn_piece(void);

void tetris_moveLeft(void);
void tetris_moveRight(void);
void tetris_rotate(void);
void tetris_softDrop(void);
void tetris_gravityStep(void);

#endif /* _TETRIS_H_ */
