#include "LPC17xx.h"
#include "GLCD.h"   // Libreria per gestire lo schermo

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

/* ==============================================
   2. VARIABILI GLOBALI (LA MEMORIA)
   ============================================== */
// Questa è la griglia: 0 = vuoto, 1 = occupato
// volatile serve a dire al compilatore che questa memoria cambia spesso
volatile int board[ROWS][COLS]; 

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