#include "LPC17xx.h"
#include "GLCD.h"   // Libreria per gestire lo schermo
#include "tetris.h"
#include <stdio.h>

/* ==============================================
   1. DEFINIZIONE DELLE COSTANTI
   ============================================== */
// Il campo è 20 righe x 10 colonne
#define ROWS 20      
#define COLS 10      
#define BLOCK_SIZE 15

// Colori 
#define Black 0x0000
#define White 0xFFFF
#define Red   0xF800

#define COLOR_I  0x07FF   // Ciano
#define COLOR_O  0xFFE0   // Giallo
#define COLOR_T  0x780F   // Viola
#define COLOR_J  0x001F   // Blu
#define COLOR_L  0xFD20   // Arancione
#define COLOR_S  0x07E0   // Verde
#define COLOR_Z  0xF800   // Rosso



// Questa è la griglia: 0 = vuoto, 1 = occupato
// volatile serve a dire al compilatore che questa memoria cambia spesso
volatile int board[ROWS][COLS]; 
volatile GameState gameState = GAME_PAUSED;
static uint8_t firstStart = 1;
volatile uint32_t score = 0;
volatile uint32_t high_score = 0;

// ======= Stato del pezzo corrente =======
static int cur_id  = 0;   // 0..6
static int cur_r   = 0;   // riga top-left della 4x4
static int cur_c   = 3;   // colonna top-left (3 -> centrato circa)
static int cur_rot = 0;   // 0..3

// Evento caduta (settato dal timer, consumato nel main)
volatile uint8_t gravity_event = 0;

// Soft drop flag (settato da joystick down)
volatile uint8_t softdrop_on = 0;
volatile uint8_t key2_event  = 0;
volatile uint8_t key1_event  = 0;
volatile int last_cleared = 0; //definita qui per essere visibile nella watch in debug
volatile uint8_t score_dirty = 0;

static uint16_t rng = 0xACE1u;

static uint8_t rand7(void)
{
    rng = (rng >> 1) ^ (-(rng & 1u) & 0xB400u);
    return (uint8_t)(rng % 7);
}




// TETRAMINI
// Usiamo uint8_t per risparmiare memoria (basta 1 byte per 0/1)
// Struttura: [7 pezzi] [4 righe] [4 colonne]

const uint8_t PIECES[7][4][4] = {
    // 0: I (Linea) - Occupa tutta la larghezza 4
    {
        {0,0,0,0},
        {1,1,1,1},
        {0,0,0,0},
        {0,0,0,0}
    },
    // 1: O (Cubo) - Sta al centro
    {
        {0,0,0,0},
        {0,1,1,0},
        {0,1,1,0},
        {0,0,0,0}
    },
    // 2: T - La T rovesciata
    {
        {0,0,0,0},
        {1,1,1,0},
        {0,1,0,0},
        {0,0,0,0}
    },
    // 3: L
    {
        {0,0,0,0},
        {1,1,1,0},
        {1,0,0,0},
        {0,0,0,0}
    },
    // 4: J
    {
        {0,0,0,0},
        {1,1,1,0},
        {0,0,1,0},
        {0,0,0,0}
    },
    // 5: S
    {
        {0,0,0,0},
        {0,1,1,0},
        {1,1,0,0},
        {0,0,0,0}
    },
    // 6: Z
    {
        {0,0,0,0},
        {1,1,0,0},
        {0,1,1,0},
        {0,0,0,0}
    }
};

const uint16_t PIECE_COLORS[7] = {
    COLOR_I,
    COLOR_O,
    COLOR_T,
    COLOR_J,
    COLOR_L,
    COLOR_S,
    COLOR_Z
};


 //FUNZIONI DI DISEGNO
void update_score(void)
{
  char buf[16];

  // pulisco la riga (spazi) per evitare residui di cifre vecchie
  GUI_Text(160, 80, (uint8_t *)"        ", Black, Black);
	GUI_Text(160, 120, (uint8_t *)"        ", Black, Black);

  sprintf(buf, "%lu", (unsigned long)score);
  GUI_Text(160, 80, (uint8_t *)buf, Red, Black);
	
	sprintf(buf, "%lu", (unsigned long)high_score);
  GUI_Text(160, 120, (uint8_t *)buf, Red, Black);
}

// Funzione per inizializzare il gioco
void Init_Game_Graphics(void) {
 
    LCD_Clear(Black); // sfondo nero
    
    // Calcoliamo i limiti del campo 
    int limit_X = COLS * BLOCK_SIZE; // Larghezza = 10 * 15 = 150 pixel
    int limit_Y = ROWS * BLOCK_SIZE; // Altezza   = 20 * 15 = 300 pixel
    
    // 2. Disegniamo il "SECCHIO" usando le linee
    // Sintassi: LCD_DrawLine(x_inizio, y_inizio, x_fine, y_fine, colore)
    
    // Linea Verticale SINISTRA (da 0,0 a 0,300)
    LCD_DrawLine(0, 0, 0, limit_Y, White);
    
    // Linea Verticale DESTRA (da 150,0 a 150,300)
    LCD_DrawLine(limit_X, 0, limit_X, limit_Y, White);
    
    // Linea Orizzontale BASSO (da 0,300 a 150,300)
    LCD_DrawLine(0, limit_Y, limit_X, limit_Y, White);

    // 3. Scriviamo il testo laterale
    GUI_Text(160, 20, (uint8_t *) "TETRIS", White, Red);
    GUI_Text(160, 60, (uint8_t *) "Score:", White, Black);
    GUI_Text(160, 80, (uint8_t *) "0", Red, Black);
		GUI_Text(160, 100, (uint8_t *)"Highest:", White, Black);
    GUI_Text(160, 120, (uint8_t *) "0", Red, Black);
		GUI_Text(30, 180, (uint8_t *) " Press key1 ", White, Red);
		GUI_Text(30, 200, (uint8_t *) " to start ", White, Red);
}

// Funzione per resettare la matrice a zero
void Reset_Board(void) {
    int r, c;
    for(r=0; r<ROWS; r++) {
        for(c=0; c<COLS; c++) {
            board[r][c] = 0; // Tutto vuoto
        }
    }
}

void Draw_Block(int r, int c, uint16_t color) {
    int x0 = c * BLOCK_SIZE;
    int y0 = r * BLOCK_SIZE;
    int i;

    // Disegniamo righe orizzontali
    // Partiamo da i=1 e finiamo a BLOCK_SIZE-2 per lasciare il bordino nero attorno
    for (i = 1; i < BLOCK_SIZE - 1; i++) {
        // Disegna una linea da sinistra (x0+1) a destra (x0+BLOCK_SIZE-1) all'altezza y0+i
        LCD_DrawLine(x0 + 1, y0 + i, x0 + BLOCK_SIZE - 1, y0 + i, color);
    }
}

// DISEGNA UN PEZZO INTERO
// r, c: Coordinate dell'angolo in alto a sinistra della scatola 4x4
// piece_id: Quale pezzo disegnare (0-6)
void Draw_Piece(int r, int c, int piece_id) {
    int i, j;
    
    // Scorre le 4 righe del pezzo
    for (i = 0; i < 4; i++) {
        // Scorre le 4 colonne del pezzo
        for (j = 0; j < 4; j++) {
            
            // Se nella definizione del pezzo c'è un 1...
            if (PIECES[piece_id][i][j] == 1) {
                // ...disegna un blocco nella posizione corrispondente
                // r+i = riga base + riga interna del pezzo
                // c+j = colonna base + colonna interna del pezzo
                Draw_Block(r + i, c + j, PIECE_COLORS[piece_id]);
            }
        }
    }
}

//mostra un nuovo tetramino
void spawn_piece(void)
{
	LPC_TIM0->TC = 0;
  cur_id  = rand7();
  cur_rot = 0;
	cur_r   = -1;     
  cur_c   = 3;

  // lose condition: se non posso piazzarlo già all'inizio
  if (!can_place(cur_r, cur_c, cur_id, cur_rot)) {
		gameState = GAME_OVER;
    GUI_Text(160, 140, (uint8_t *) "GAME OVER", Red, Black);
    return;
  }

    draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
}



//funzioni di gioco
void toggle_pause(){
	if(firstStart || gameState == GAME_OVER){
		if(gameState == GAME_OVER){
			Reset_Board();
			redraw_board();
			if(score > high_score){
				high_score = score;
			}
			score = 0;
			score_dirty = 1;
		}
		if(firstStart){
			// cancello l'avviso press k1 to start
			GUI_Text(30, 180, (uint8_t *) "            ", Black, Black);
			GUI_Text(30, 200, (uint8_t *) "           ", Black, Black);
		}
    spawn_piece();
    gameState = GAME_RUNNING;
    firstStart = 0;
		GUI_Text(160, 140, (uint8_t *) "          ", COLOR_T, Black);
		GUI_Text(160, 140, (uint8_t *) "PLAYING", COLOR_T, Black);
    return;
    }

    /* GIOCO IN CORSO ? PAUSA */
    if (gameState == GAME_RUNNING) {
			gameState = GAME_PAUSED;
			GUI_Text(160, 140, (uint8_t *) "         ", COLOR_T, Black);
			GUI_Text(160, 140, (uint8_t *) "PAUSED", COLOR_T, Black);
      return;
    }

    /* PAUSA ? RIPRENDI */
    if (gameState == GAME_PAUSED) {
      gameState = GAME_RUNNING;
			GUI_Text(160, 140, (uint8_t *) "          ", COLOR_T, Black);
			GUI_Text(160, 140, (uint8_t *) "PLAYING", COLOR_T, Black);
      return;
    }

    /* GAME OVER ? RESTART 
    if (gameState == GAME_OVER) {
				firstStart = 1;
        Reset_Board();
				redraw_board();
        spawn_piece();
        gameState = GAME_RUNNING;
    }
		*/
	}

	
//funzioni di movimento
	
	static uint8_t piece_cell(int id, int rot, int i, int j)
{
    switch (rot & 3) {
        case 0: return PIECES[id][i][j];
        case 1: return PIECES[id][3 - j][i];
        case 2: return PIECES[id][3 - i][3 - j];
        default:return PIECES[id][j][3 - i];
    }
}

	
	static int can_place(int r0, int c0, int id, int rot){
		int i = 0;
		int j = 0;
    for (i=0; i<4; i++) {
        for (j=0; j<4; j++) {
            if (piece_cell(id, rot, i, j)) {
                int r = r0 + i;
                int c = c0 + j;

                // fuori dal campo
                if (c < 0 || c >= COLS) return 0;
                if (r >= ROWS) {
									return 0;
								}

                // sopra il bordo top: consentito (spawn), ma non checkare board[-1]
                if (r >= 0) {
                    if (board[r][c] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

static void draw_piece_at(int r0, int c0, int id, int rot, uint16_t color){
    int i = 0;
		int j = 0;
    for (i=0; i<4; i++) {
        for (j=0; j<4; j++) {
            if (piece_cell(id, rot, i, j)) {
                int r = r0 + i;
                int c = c0 + j;
                if (r >= 0) Draw_Block(r, c, color);
            }
        }
    }
}

static void erase_piece_at(int r0, int c0, int id, int rot)
{
    draw_piece_at(r0, c0, id, rot, Black);
}

static void lock_piece(void){
    int i = 0;
		int j = 0;
    for (i=0; i<4; i++) {
        for (j=0; j<4; j++) {
            if (piece_cell(cur_id, cur_rot, i, j)) {
                int r = cur_r + i;
                int c = cur_c + j;
                if (r >= 0 && r < ROWS && c >= 0 && c < COLS) {
                    board[r][c] = cur_id + 1; // salva tipo (1..7)
                }
            }
        }
    }
}

static void redraw_board(void) {
    // pulisci area gioco (solo interno, non il bordo)
		int r = 0;
		int c = 0;
    for (r=0; r<ROWS; r++) {
        for (c=0; c<COLS; c++) {
            Draw_Block(r, c, Black);
        }
    }

    // ridisegna blocchi fissi
    for (r=0; r<ROWS; r++) {
        for (c=0; c<COLS; c++) {
            if (board[r][c] != 0) {
                int id = board[r][c] - 1;
                Draw_Block(r, c, PIECE_COLORS[id]);
            }
        }
    }
}

void tetris_moveLeft(void){
  if (gameState != GAME_RUNNING) return;
		LPC_TIM0->TC = 0;

    if (can_place(cur_r, cur_c - 1, cur_id, cur_rot)) {
        erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
        cur_c--;
        draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
    }
}

void tetris_moveRight(void)
{
  if (gameState != GAME_RUNNING) return;
	LPC_TIM0->TC = 0;
	
  if (can_place(cur_r, cur_c + 1, cur_id, cur_rot)) {
		erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
    cur_c++;
    draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
  }
}

void tetris_rotate(void){
  if (gameState != GAME_RUNNING) return;
	LPC_TIM0->TC = 0;
  int new_rot = (cur_rot + 1) & 3;

   // semplice wall-kick minimo: prova in posto, poi sposta di 1 a sx o dx
    if (can_place(cur_r, cur_c, cur_id, new_rot) ||
        can_place(cur_r, cur_c - 1, cur_id, new_rot) ||
        can_place(cur_r, cur_c + 1, cur_id, new_rot)) {

        erase_piece_at(cur_r, cur_c, cur_id, cur_rot);

        if (!can_place(cur_r, cur_c, cur_id, new_rot)) {
            if (can_place(cur_r, cur_c - 1, cur_id, new_rot)) cur_c--;
            else if (can_place(cur_r, cur_c + 1, cur_id, new_rot)) cur_c++;
        }

        cur_rot = new_rot;
        draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
    }
}

void tetris_gravityStep(void)
{
    if (gameState != GAME_RUNNING) return;

    if (can_place(cur_r + 1, cur_c, cur_id, cur_rot)) {
        erase_piece_at(cur_r, cur_c, cur_id, cur_rot);
        cur_r++;
        draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);
    } else {
			lock_piece();
			last_cleared = clear_lines();
			if (last_cleared > 0) {
        redraw_board();   // necessario per vedere lo shift
				score++;
				if(score > high_score){
					high_score = score;
				}
				score_dirty = 1;
    }
    spawn_piece();
}

}

void tetris_softDrop(void)
{
    // chiamala nel main ogni ciclo (o nel RIT) quando softdrop_on=1
    if (softdrop_on) {
        tetris_gravityStep(); // un passo extra
    }
}

void tetris_hardDrop(void)
{
    if (gameState != GAME_RUNNING) return;

    // Cancella il pezzo dalla vecchia posizione (grafica)
    erase_piece_at(cur_r, cur_c, cur_id, cur_rot);

    // Scendi finché puoi
    while (can_place(cur_r + 1, cur_c, cur_id, cur_rot)) {
        cur_r++;
    }

    // Disegna nella posizione finale
    draw_piece_at(cur_r, cur_c, cur_id, cur_rot, PIECE_COLORS[cur_id]);

    // Diventa blocco fisso
    lock_piece();

    int n = clear_lines();
		if(n>0) redraw_board();
		
    // Nuovo pezzo
    //spawn_piece();
}

// ricalcola la nuova board e conta il numero di righe da cancellare
// se è >0, la board viene sovrascritta 
int clear_lines(void)
{
    int r, c;
    int write_r = ROWS - 1;
    int cleared = 0;

    for (r = ROWS - 1; r >= 0; r--) {
        int full = 1;
        for (c = 0; c < COLS; c++) {
            if (board[r][c] == 0) { full = 0; break; }
        }

        if (full) {
            cleared++;
        } else {
            if (write_r != r) {
                for (c = 0; c < COLS; c++) {
                    board[write_r][c] = board[r][c];
                }
            }
            write_r--;
        }
    }

    for (r = write_r; r >= 0; r--) {
        for (c = 0; c < COLS; c++) board[r][c] = 0;
    }

    return cleared;
}



