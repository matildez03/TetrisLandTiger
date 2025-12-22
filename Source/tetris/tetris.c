#include "LPC17xx.h"
#include "GLCD.h"   // Libreria per gestire lo schermo
#include "tetris.h"
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

/* ==============================================
   2. VARIABILI GLOBALI (LA MEMORIA)
   ============================================== */
// Questa è la griglia: 0 = vuoto, 1 = occupato
// volatile serve a dire al compilatore che questa memoria cambia spesso
volatile int board[ROWS][COLS]; 

volatile GameState gameState = GAME_PAUSED;
/* ==============================================
   4. DEFINIZIONE DEI PEZZI (TETRAMINI)
   ============================================== */
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





/* ==============================================
   3. FUNZIONI DI DISEGNO
   ============================================== */

// Funzione per inizializzare il gioco
void Init_Game_Graphics(void) {
    // 1. Puliamo lo schermo (Sfondo nero)
    LCD_Clear(Black);
    
    // Calcoliamo i limiti del campo per comodità
    // Larghezza = 10 * 15 = 150 pixel
    // Altezza   = 20 * 15 = 300 pixel
    int limit_X = COLS * BLOCK_SIZE; 
    int limit_Y = ROWS * BLOCK_SIZE;
    
    // 2. Disegniamo il "SECCHIO" usando le linee
    // Sintassi: LCD_DrawLine(x_inizio, y_inizio, x_fine, y_fine, colore)
    
    // Linea Verticale SINISTRA (da 0,0 a 0,300)
    LCD_DrawLine(0, 0, 0, limit_Y, White);
    
    // Linea Verticale DESTRA (da 150,0 a 150,300)
    LCD_DrawLine(limit_X, 0, limit_X, limit_Y, White);
    
    // Linea Orizzontale BASSO (da 0,300 a 150,300)
    LCD_DrawLine(0, limit_Y, limit_X, limit_Y, White);

    // 3. Scriviamo il testo laterale
    GUI_Text(160, 20, (uint8_t *) "TETRIS", White, Black);
    GUI_Text(160, 60, (uint8_t *) "Score:", White, Black);
    GUI_Text(160, 80, (uint8_t *) "0", Red, Black);
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

/* ==============================================
   5. DISEGNA UN PEZZO INTERO
   ============================================== */
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


void spawn_piece(){
	int i = LPC_TIM0->TC % 7;
	Reset_Board();
	Draw_Piece(0,5,i);
}

//funzioni di movimento

void tetris_softDrop(void)
{
    // per ora anche vuota va bene
}


void tetris_moveLeft(void){}
void tetris_moveRight(void){}
void tetris_rotate(void){}
void tetris_gravityStep(void){}