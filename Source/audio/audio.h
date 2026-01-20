#pragma once
#include <stdint.h>

typedef struct {
    uint16_t freq;      // Hz (0 = pausa)
    uint16_t dur_ms;    // durata in ms
} NoteEvent;



// chiamata ad ogni interrupt del timer audio (Fs)
uint16_t audio_next_sample_10bit(void);

// per cambiare nota/volume dall'esterno
void audio_set_freq(uint16_t freq_hz);       // 0 = mute
void audio_set_volume(uint16_t vol_0_1023);  // 30..120 consigliato
void audio_init(uint32_t sample_rate_hz);
void audio_set_volume(uint16_t vol_0_1023);    // ampiezza (consiglio 30..120)
void audio_play_tone(uint16_t freq_hz, uint16_t dur_ms);
void audio_play_score(const NoteEvent *score, uint32_t len);
void audio_silence(void);

// effetti sonori
void sfx_piece_drop(void);
void sfx_game_over(void);

// Tema
void play_tetris_theme(void);


//--------------------------------------------------------------------------------------//
// NOTE
// Frequenze (Hz) in ottava bassa / media (non acute)
#define NOTE_C3 131
#define NOTE_D3 147
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_G3 196
#define NOTE_A3 220
#define NOTE_B3 247

#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494

