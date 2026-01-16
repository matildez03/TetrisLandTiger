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

typedef enum { 
PU_NONE=0, PU_CLEAR_HALF=1, PU_SLOW=2 
} 
PuType; //power ups
//uint8_t pu[H][W];    // 0 none, 1 clear_half, 2 slow

/* Stato pubblico (letto dalla grafica e dal main) */
extern volatile int board[ROWS][COLS];
extern volatile uint8_t pu[ROWS][COLS];   // 0 none, 1 clear_half, 2 slow
extern volatile GameState gameState;

extern volatile uint8_t gravity_event;
extern volatile uint8_t softdrop_on; //down_activate
extern volatile uint8_t key1_event;
extern volatile uint8_t key2_event;
extern volatile uint8_t right_activate;
extern volatile uint8_t left_activate;
extern volatile uint8_t up_activate;
extern volatile uint8_t trig_clear_half;
extern volatile uint8_t trig_slow;

extern volatile uint32_t score;
extern volatile uint32_t high_score;
extern volatile uint32_t row_count;
extern volatile uint8_t score_dirty;
extern volatile int last_cleared;

/* Dati pezzi (usati anche dalla grafica) */
extern const uint8_t  PIECES[7][4][4];
extern const uint16_t PIECE_COLORS[8];

// RNG per random
extern uint16_t rng;

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
void activate_powerup_half(void);
void activate_malus_line();

int clear_lines(void);
static uint8_t board_is_empty(void);
static void spawn_powerup(void);

#endif /* _TETRIS_H_ */
