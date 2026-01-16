#ifndef _TETRIS_COLORS_H_
#define _TETRIS_COLORS_H_

#include <stdint.h>

/* RGB565 */
#define COLOR_I  0x07FF   /* Ciano   */
#define COLOR_O  0xFFE0   /* Giallo  */
#define COLOR_T  0x780F   /* Viola   */
#define COLOR_J  0x001F   /* Blu     */
#define COLOR_L  0xFD20   /* Arancio */
#define COLOR_S  0x07E0   /* Verde   */
#define COLOR_Z  0xF800   /* Rosso   */

#define COLOR_PU_CLEAR_HALF  0xF81F  /* Magenta (fucsia) */
#define COLOR_PU_SLOW        0xFFFF  /* Bianco */
#define COLOR_MALUS  0x4010 // viola scuro

#define BG_COLOR 0x4208   /* grigio scuro */
// #define BG_COLOR 0x2104   /* grigio molto scuro */
//#define GRAY 0x4208   /* grigio scuro */


/* Colori base UI */
#define Black 0x0000
#define White 0xFFFF
#define Red   0xF800

#endif /* _TETRIS_COLORS_H_ */
