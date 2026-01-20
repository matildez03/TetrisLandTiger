#include "LPC17xx.h"
#include "tetris.h"
#include "tetris_gfx.h"
#include "tetris_colors.h"
#include <stdio.h>
#include "../adc/adc.h"


/* ===== Board e stato ===== */
volatile int board[ROWS][COLS];
volatile uint8_t pu[ROWS][COLS];   // 0 none, 1 clear_half, 2 slow
volatile GameState gameState = GAME_PAUSED;
static uint8_t firstStart = 1;

volatile uint32_t score = 0;
volatile uint32_t high_score = 0;
volatile uint32_t row_count = 0;

volatile uint8_t gravity_event = 0;
volatile uint8_t softdrop_on   = 0;
volatile uint8_t key1_event    = 0;
volatile uint8_t key2_event    = 0;
volatile uint8_t right_activate = 0;
volatile uint8_t left_activate = 0;
volatile uint8_t up_activate = 0;
volatile uint8_t trig_clear_half = 0;
volatile uint8_t trig_slow = 0;

volatile int last_cleared      = 0;
volatile uint8_t score_dirty   = 0;
static int next_powerup_at = 5;
static int next_malus_at = 10;


/* ===== Stato pezzo corrente ===== */
static int cur_id  = 0;
static int cur_r   = 0;
static int cur_c   = 3;
static int cur_rot = 0;

// RNG per random
uint16_t rng = 0xACE1u;

void seed_rng_once(void){
	rng ^= (uint16_t)LPC_TIM0->TC;
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


const uint16_t PIECE_COLORS[8] = {
    COLOR_I, COLOR_O, COLOR_T, COLOR_J, COLOR_L, COLOR_S, COLOR_Z, COLOR_MALUS
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
void Reset_PU(void){
	int r;
	int c;
    
	for (r = 0; r < ROWS; r++) {
		for (c = 0; c < COLS; c++) {
			pu[r][c] = 0;
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
				sfx_piece_drop();
				score = score +10;
				if (score > high_score) high_score = score;
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
        GUI_Text(160, 200, (uint8_t *)"GAME OVER", White, Red);
								sfx_game_over();
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
												Reset_PU();
            redraw_board();

            if (score > high_score) high_score = score;
            score = 0;
            score_dirty = 1;
        }

        if (firstStart) {
            seed_rng_once();
        }

								play_tetris_theme();

        spawn_piece();
        gameState = GAME_RUNNING;
        firstStart = 0;

								GUI_Text(160, 180, (uint8_t *)"          ", White, BG_COLOR);
								GUI_Text(160, 200, (uint8_t *)"          ", White, BG_COLOR);
								GUI_Text(160, 220, (uint8_t *)"          ",   White, BG_COLOR);
								GUI_Text(170, 200, (uint8_t *)"PLAYING",   White,COLOR_T);
        return;
    }

    if (gameState == GAME_RUNNING) {
        gameState = GAME_PAUSED;
								audio_silence();
        GUI_Text(160, 200, (uint8_t *)"          ", COLOR_T, BG_COLOR);
        GUI_Text(170, 200, (uint8_t *)"PAUSED",   White, COLOR_T);
        return;
    }

    if (gameState == GAME_PAUSED) {
        gameState = GAME_RUNNING;
								play_tetris_theme();
        GUI_Text(160, 200, (uint8_t *)"        ", COLOR_T, BG_COLOR);
        GUI_Text(170, 200, (uint8_t *)"PLAYING",   White,COLOR_T);
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
				audio_music_tick(1);
    if (can_place(cur_r + 1, cur_c, cur_id, cur_rot)) {
        gfx_erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
        cur_r++;
        gfx_draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
        return;
    }

    lock_piece();
    last_cleared = clear_lines();
    if (last_cleared > 0) {
					if (board_is_empty()) {
        score += 600;          // tetris 
					}else{
							 score = score + (100 * last_cleared);
					}
					if (score > high_score) high_score = score;
					
					redraw_board();
    }
				
				score_dirty = 1;

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
					if (board_is_empty()) {
        score += 600;          // tetris 
					}else{
							 score = score + (100 * n);
					}
					if (score > high_score) high_score = score;
					redraw_board();
    }
				score_dirty = 1;
    spawn_piece();
}

// funzione per pulire le righe completate, ritorna il numero di righe cancellate
int clear_lines(void){ 
    int cleared = 0;
    int r, c, i;

    trig_clear_half = 0;
    trig_slow = 0;

    for (r = ROWS - 1; r >= 0; r--) {
        int full = 1;
        for (c = 0; c < COLS; c++) {
            if (board[r][c] == 0) { 
													if(pu[r][c] == 0){
														full = 0; break;
													} 
												}				
								}
								

        if (full) {
            // 1) rileva powerup presenti nella riga r (prima di spostare tutto)
            for (c = 0; c < COLS; c++) {
                if (pu[r][c] == PU_CLEAR_HALF) trig_clear_half = 1;
                if (pu[r][c] == PU_SLOW)       trig_slow = 1;
            }

            cleared++;

            // 2) shift down di board + pu
            for (i = r; i > 0; i--) {
                for (c = 0; c < COLS; c++) {
                    board[i][c] = board[i - 1][c];
                    pu[i][c]    = pu[i - 1][c];
                }
            }

            // 3) clear top row
            for (c = 0; c < COLS; c++) {
                board[0][c] = 0;
                pu[0][c]    = PU_NONE;
            }

            r++; // ricontrolla stessa riga
        }
    }

    row_count += cleared;

    while (row_count >= next_powerup_at) {
        spawn_powerup();
        next_powerup_at += 5;
    }

    while (row_count >= next_malus_at) {
        activate_malus_line();
        next_malus_at += 10;
    }

    return cleared;
}


static uint8_t board_is_empty(void)
{
    int r, c;
    for (r = 0; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
            if (board[r][c] != 0) return 0;
        }
    }
    return 1;
}

// POTENZIOMETRO
extern volatile uint16_t AD_current;

uint32_t compute_mr0_from_adc(uint16_t adc, uint8_t softdrop){
    if (adc < 50) adc = 0;   // dead-zone

    uint32_t speed_milli = 1000UL + (4000UL * (uint32_t)adc) / 0xFFF; // 1000..5000
    if (softdrop) speed_milli *= 2;

    uint64_t num = 25000000ULL * 1000ULL;   // 64-bit, NO overflow
    return (uint32_t)(num / speed_milli);
}

// POWER UP

static void spawn_powerup(void)
{
    //celle occupate
 int n = 0;
	int y;
	int x;
   
 for (y = 0; y < ROWS; y++){
		for (x = 0; x < COLS; x++){
			if (board[y][x] != 0){
				n++;
			}
		}
	}
	
	if (n == 0) return;

	// indice casuale 0..n-1 
	int k = rand() % n;

 // trova la k-esima cella occupata
	for (y = 0; y < ROWS; y++) {
		for (x = 0; x < COLS; x++) {
			if (board[y][x] != 0) {
				if (k == 0) {
					pu[y][x] = (rand() % 2) ? PU_CLEAR_HALF : PU_SLOW;
					return;
				}
				k--;
			}
		}
	}
}
void activate_powerup_half(void) {

    int start_row = ROWS / 2; // Metà inferiore
    int cleared_count = 0;

    // Cancella le righe
		int r, c, k;
    for (r = start_row; r < ROWS; r++) {
        for (c = 0; c < COLS; c++) {
									board[r][c] = 0;
									pu[0][0] = 0;
        }
        cleared_count++;
    }
				
				// Shift delle linee
				for(k = 0; k < start_row ; k++) {
					for(c = 0; c < COLS; c++) {
						board[k+start_row][c] = board[k][c];
						pu[k+start_row][c] = pu[k][c];
						pu[k][c] = 0;
						board[k][c] = 0;
					}
				}

    // Calcolo Punteggio Speciale [cite: 59]
    // "groups of 4 lines" -> Punti Tetris (es. 610)
    // "last group will contain less than 4" -> Punti normali (es. 100/riga)
    
    int groups_of_4 = cleared_count / 4;
    int remainder = cleared_count % 4;
    
    score += (groups_of_4 * 610); // Punti per i gruppi da 4
    score += (remainder * 100);   // Punti per le righe rimanenti
    
				if(score> high_score){
					high_score = score;
				}
				
    row_count += cleared_count; 
    
				score_dirty = 1;
				redraw_board();
}

void activate_powerup_slow(void) {
    //gestita nel main
}

// MALUS
void activate_malus_line(void) {
    int r, c;
    
    // Check Overflow
    for(c = 0; c < COLS; c++) {
        if (board[0][c] != 0) {
									gameState = GAME_OVER;
        /* Messaggio grafico */
        extern void GUI_Text(int x, int y, uint8_t *text, uint16_t fg, uint16_t bg);
        GUI_Text(160, 200, (uint8_t *)"GAME OVER", White, Red);
								sfx_game_over();
									return;
        }
    }

    // Shift delle linee verso l'alto
    for(r = 0; r < ROWS - 1; r++) {
       for(c = 0; c < COLS; c++) {
           board[r][c] = board[r+1][c];
       }
    }
    
    // Genera la linea malus
    int last_row = ROWS - 1;
    for(c = 0; c < COLS; c++) board[last_row][c] = 0;
    
    int filled = 0;
    while(filled < 7) {
        int col = rand() % COLS;
        if (board[last_row][col] == 0) {
									board[last_row][col] = 8; // 8: id dei blocchi del malus
             filled++;
        }
    }
    
    redraw_board();
}



