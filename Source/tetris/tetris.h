#ifndef _TETRIS_H_
#define _TETRIS_H_

#include <stdint.h>

/* Campo */
#define ROWS 20
#define COLS 10

typedef enum {
    GAME_PAUSED = 0,
    GAME_RUNNING,
    GAME_OVER
} GameState;

/* Stato pubblico (letto dalla grafica e dal main) */
extern volatile int board[ROWS][COLS];
extern volatile GameState gameState;

extern volatile uint8_t gravity_event;
extern volatile uint8_t softdrop_on; //down_activate
extern volatile uint8_t key1_event;
extern volatile uint8_t key2_event;
extern volatile uint8_t right_activate;
extern volatile uint8_t left_activate;
extern volatile uint8_t up_activate;

extern volatile uint32_t score;
extern volatile uint32_t high_score;
extern volatile uint8_t score_dirty;
extern volatile int last_cleared;

/* Dati pezzi (usati anche dalla grafica) */
extern const uint8_t  PIECES[7][4][4];
extern const uint16_t PIECE_COLORS[7];

// rotazione dei pezzi
uint8_t piece_cell(int id, int rot, int i, int j);

/* API gioco */
void seed_rng_once(void);
void Reset_Board(void);

void spawn_piece(void);
void toggle_pause(void);

void tetris_moveLeft(void);
void tetris_moveRight(void);
void tetris_rotate(void);
void tetris_gravityStep(void);
void tetris_hardDrop(void);

int clear_lines(void);

#endif /* _TETRIS_H_ */
