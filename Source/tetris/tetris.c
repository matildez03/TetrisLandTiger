#include "LPC17xx.h"
#include "tetris.h"
#include "tetris_gfx.h"
#include "tetris_colors.h"
#include <stdio.h>

/* ===== Board e stato ===== */
volatile int board[ROWS][COLS];
volatile GameState gameState = GAME_PAUSED;
static uint8_t firstStart = 1;

volatile uint32_t score = 0;
volatile uint32_t high_score = 0;

volatile uint8_t gravity_event = 0;
volatile uint8_t softdrop_on   = 0;
volatile uint8_t key1_event    = 0;
volatile uint8_t key2_event    = 0;
volatile int last_cleared      = 0;
volatile uint8_t score_dirty   = 0;

/* ===== Stato pezzo corrente ===== */
static int cur_id  = 0;
static int cur_r   = 0;
static int cur_c   = 3;
static int cur_rot = 0;

// RNG per random
static uint16_t rng = 0xACE1u;

void seed_rng_once(void){
	rng ^= (uint16_t)LPC_TIM1->TC;
	rng ^= (uint16_t)LPC_RIT->RICOUNTER;
	if (rng == 0) rng = 0xACE1u;
}

static uint8_t rand7(void){
	rng = (rng >> 1) ^ (-(rng & 1u) & 0xB400u);
  return (uint8_t)(rng % 7);
}

// TETRAMINI -------------------------

const uint8_t PIECES[7][4][4] = {
    /* I */
    {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}},
    /* O */
    {{0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0}},
    /* T */
    {{0,0,0,0},{1,1,1,0},{0,1,0,0},{0,0,0,0}},
    /* L */
    {{0,0,0,0},{1,1,1,0},{1,0,0,0},{0,0,0,0}},
    /* J */
    {{0,0,0,0},{1,1,1,0},{0,0,1,0},{0,0,0,0}},
    /* S */
    {{0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0}},
    /* Z */
    {{0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}}
};

const uint16_t PIECE_COLORS[7] = {
    COLOR_I, COLOR_O, COLOR_T, COLOR_J, COLOR_L, COLOR_S, COLOR_Z
};

/* ===== funzioni interne logica ===== */

uint8_t piece_cell(int id, int rot, int i, int j)
{
    switch (rot & 3) {
        case 0: return PIECES[id][i][j];
        case 1: return PIECES[id][3 - j][i];
        case 2: return PIECES[id][3 - i][3 - j];
        default:return PIECES[id][j][3 - i];
    }
}

static int can_place(int r0, int c0, int id, int rot){
	int i;
	int j;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			if (piece_cell(id, rot, i, j)) {
				int r = r0 + i;
				int c = c0 + j;
				if (c < 0 || c >= COLS) return 0;
				if (r >= ROWS) return 0;
				if (r >= 0) {
					if (board[r][c] != 0) return 0;
				}
			}
		}
	}
	return 1;
}

void Reset_Board(void){
	int r;
	int c;
    
	for (r = 0; r < ROWS; r++) {
		for (c = 0; c < COLS; c++) {
			board[r][c] = 0;
		}
	}
}

static void lock_piece(void){
	int i;
	int j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (piece_cell(cur_id, cur_rot, i, j)) {
                int r = cur_r + i;
                int c = cur_c + j;
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                    board[r][c] = cur_id + 1;
                }
            }
        }
    }
}

/* ===== API gioco ===== */

void spawn_piece(void){
    LPC_TIM0->TC = 0;
    cur_id  = rand7();
    cur_rot = 0;
    cur_r   = -1;
    cur_c   = 3;

    if (!can_place(cur_r, cur_c, cur_id, cur_rot)) {
        gameState = GAME_OVER;
        /* Messaggio grafico */
        extern void GUI_Text(int x, int y, uint8_t *text, uint16_t fg, uint16_t bg);
        GUI_Text(160, 140, (uint8_t *)"GAME OVER", White, Red);
        return;
    }

    gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
}

void toggle_pause(void)
{
    extern void GUI_Text(int x, int y, uint8_t *text, uint16_t fg, uint16_t bg);

    if (firstStart || gameState == GAME_OVER) {
        if (gameState == GAME_OVER) {
            Reset_Board();
            redraw_board();

            if (score > high_score) high_score = score;
            score = 0;
            score_dirty = 1;
        }

        if (firstStart) {
            seed_rng_once();
        }

        spawn_piece();
        gameState = GAME_RUNNING;
        firstStart = 0;

       	GUI_Text(160, 180, (uint8_t *)"          ", White, BG_COLOR);
								GUI_Text(160, 200, (uint8_t *)"          ", White, BG_COLOR);
								GUI_Text(160, 220, (uint8_t *)"          ",   White, BG_COLOR);
								GUI_Text(170, 140, (uint8_t *)"PLAYING",   White,COLOR_T);
        return;
    }

    if (gameState == GAME_RUNNING) {
        gameState = GAME_PAUSED;
        GUI_Text(170, 140, (uint8_t *)"        ", COLOR_T, BG_COLOR);
        GUI_Text(170, 140, (uint8_t *)"PAUSED",   White, COLOR_T);
        return;
    }

    if (gameState == GAME_PAUSED) {
        gameState = GAME_RUNNING;
        GUI_Text(170, 140, (uint8_t *)"        ", COLOR_T, BG_COLOR);
        GUI_Text(170, 140, (uint8_t *)"PLAYING",   White,COLOR_T);
        return;
    }
}

void tetris_moveLeft(void)
{
    if (gameState != GAME_RUNNING) return;
    LPC_TIM0->TC = 0;

    if (can_place(cur_r, cur_c - 1, cur_id, cur_rot)) {
        gfx_erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
        cur_c--;
        gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
    }
}

void tetris_moveRight(void)
{
    if (gameState != GAME_RUNNING) return;
    LPC_TIM0->TC = 0;

    if (can_place(cur_r, cur_c + 1, cur_id, cur_rot)) {
        gfx_erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
        cur_c++;
        gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
    }
}

void tetris_rotate(void)
{
    if (gameState != GAME_RUNNING) return;
    LPC_TIM0->TC = 0;

    int new_rot = (cur_rot + 1) & 3;

    /* mini wall-kick */
    if (can_place(cur_r, cur_c,     cur_id, new_rot) ||
        can_place(cur_r, cur_c - 1, cur_id, new_rot) ||
        can_place(cur_r, cur_c + 1, cur_id, new_rot)) {

        gfx_erase_piece_at(cur_r, cur_c, cur_id, cur_rot);

        if (!can_place(cur_r, cur_c, cur_id, new_rot)) {
            if (can_place(cur_r, cur_c - 1, cur_id, new_rot)) cur_c--;
            else if (can_place(cur_r, cur_c + 1, cur_id, new_rot)) cur_c++;
        }

        cur_rot = new_rot;
        gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
    }
}

void tetris_gravityStep(void)
{
    if (gameState != GAME_RUNNING) return;

    if (can_place(cur_r + 1, cur_c, cur_id, cur_rot)) {
        gfx_erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
        cur_r++;
        gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
        return;
    }

    lock_piece();
    last_cleared = clear_lines();

    if (last_cleared > 0) {
        redraw_board();
        score += (uint32_t)last_cleared;
        if (score > high_score) high_score = score;
        score_dirty = 1;
    }

    spawn_piece();
}

void tetris_hardDrop(void)
{
    if (gameState != GAME_RUNNING) return;

    gfx_erase_piece_at(cur_r, cur_c, cur_id, cur_rot);

    while (can_place(cur_r + 1, cur_c, cur_id, cur_rot)) {
        cur_r++;
    }

    gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);

    lock_piece();

    int n = clear_lines();
    if (n > 0) {
        redraw_board();
        score += (uint32_t)n;
        if (score > high_score) high_score = score;
        score_dirty = 1;
    }

    spawn_piece();
}

// funzione per pulire le righe completate, ritorna il numero di righe cancellate
int clear_lines(void){ 
	int cleared = 0;
	int r;
	int c;
	int i;
	

    for (r = ROWS - 1; r >= 0; r--) {
        int full = 1;
        for (c = 0; c < COLS; c++) {
            if (board[r][c] == 0) { full = 0; break; }
        }

        if (full) {
            cleared++;

            for (i = r; i > 0; i--) {
                for (c = 0; c < COLS; c++) {
                    board[i][c] = board[i - 1][c];
                }
            }

            for (c = 0; c < COLS; c++) board[0][c] = 0;

            r++; /* ricontrolla la stessa riga */
        }
    }

    return cleared;
}
