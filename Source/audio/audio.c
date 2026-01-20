#include "LPC17xx.h"
#include "audio.h"
#include <stdint.h>

#define SINE_N        64u
#define SINE_BITS     6u               // log2(64)
#define DAC_MID       512
#define DEFAULT_VOL   256              // 0..512 circa (512 = ampiezza piena)

// =====================
// Stato audio (DAC synthesis)
// =====================
static volatile uint32_t g_phase = 0;
static volatile uint32_t g_phase_step = 0;
static volatile uint16_t g_volume = DEFAULT_VOL;
static uint32_t g_fs = 20000;

// Tabella sinusoide 0..1023 (64 campioni), centrata su 512
static const uint16_t sinTab[SINE_N] = {
  512, 562, 611, 659, 704, 747, 786, 822,
  853, 880, 901, 916, 924, 924, 916, 901,
  880, 853, 822, 786, 747, 704, 659, 611,
  562, 512, 461, 412, 364, 319, 276, 237,
  200, 171, 147, 129, 116, 109, 107, 109,
  116, 129, 147, 171, 200, 237, 276, 319,
  364, 412, 461, 512, 562, 611, 659, 704,
  747, 786, 822, 853, 880, 901, 916, 924
};
// Nota: è una sinusoide 64-sample “liscia” e power-of-two friendly.

#define PCLK_TIMER1_HZ 25000000UL

static uint32_t timer1_get_pclk_hz(void){
    return PCLK_TIMER1_HZ;
}


static void set_phase_step(uint16_t freq_hz){
    if(freq_hz == 0 || g_fs == 0){
        g_phase_step = 0;
        return;
    }
    // phase_step = freq * 2^32 / Fs
    g_phase_step = (uint32_t)(((uint64_t)freq_hz * 0x100000000ULL) / (uint64_t)g_fs);
}

void audio_set_volume(uint16_t vol_0_512){
    // 0..512 è “comodo” perché poi dividi per 512
    if(vol_0_512 > 512) vol_0_512 = 512;
    g_volume = vol_0_512;
}

void audio_set_freq(uint16_t freq_hz){
    set_phase_step(freq_hz);
}

void audio_silence(void){
    set_phase_step(0);
}

// =====================
// TIMER1 IRQ: output 1 sample sul DAC
// =====================
void TIMER1_IRQHandler(void){
    if(LPC_TIM1->IR & 1){
        LPC_TIM1->IR = 1; // clear MR0 interrupt

        if(g_phase_step == 0){
            LPC_DAC->DACR = ((uint32_t)DAC_MID << 6);
            return;
        }

        g_phase += g_phase_step;
        uint32_t idx = g_phase >> (32 - SINE_BITS);  // 0..63

        int32_t x = (int32_t)sinTab[idx] - (int32_t)DAC_MID;
        int32_t y = (int32_t)DAC_MID + (x * (int32_t)g_volume) / 512;

        if(y < 0) y = 0;
        if(y > 1023) y = 1023;

        // DACR: VALUE in bits 15:6. (Bias bit 16 lasciato a 0)
        LPC_DAC->DACR = ((uint32_t)y << 6);
    }
}

// =====================
// Init Timer1 @ Fs
// =====================
static void timer1_init_for_fs(uint32_t fs_hz){
    if(fs_hz == 0) fs_hz = 20000;
    g_fs = fs_hz;

    LPC_SC->PCONP |= (1 << 2); // PCTIM1

    LPC_TIM1->TCR = 0x02; // reset
    LPC_TIM1->PR  = 0;

    uint32_t pclk = timer1_get_pclk_hz();

    // MR0 = (pclk/fs) - 1, con clamp minimo
    uint32_t mr0 = (pclk / fs_hz);
    if(mr0 == 0) mr0 = 1;
    LPC_TIM1->MR0 = mr0 - 1;

    LPC_TIM1->MCR = 0x03; // interrupt + reset on MR0
    NVIC_EnableIRQ(TIMER1_IRQn);
    LPC_TIM1->TCR = 0x01; // enable
}

static void dac_init(void){
    // P0.26 -> AOUT (PINSEL1 bits 21:20 = 10)
    LPC_PINCON->PINSEL1 &= ~(3u << 20);
    LPC_PINCON->PINSEL1 |=  (2u << 20);

    LPC_DAC->DACR = ((uint32_t)DAC_MID << 6);
}

void audio_init(uint32_t sample_rate_hz){
    dac_init();
    audio_set_volume(DEFAULT_VOL);
    timer1_init_for_fs(sample_rate_hz);
    audio_silence();
}

// =====================================================
// PLAYER NON-BLOCCANTE (per il tema)
// =====================================================
static const NoteEvent *m_score = 0;
static uint32_t m_len = 0;
static uint32_t m_i = 0;
static int32_t  m_left_ms = 0;
static uint8_t  m_playing = 0;
static uint8_t  m_loop = 1;

void audio_music_start(const NoteEvent *score, uint32_t len, uint8_t loop){
    m_score = score;
    m_len = len;
    m_i = 0;
    m_left_ms = 0;
    m_playing = 1;
    m_loop = loop;
}

void audio_music_stop(void){
    m_playing = 0;
    audio_set_freq(0);
}

uint8_t audio_music_is_playing(void){
    return m_playing;
}

// Chiamala dal tick del gioco: es. ogni 20ms -> audio_music_tick(20);
void audio_music_tick(uint32_t dt_ms){
    if(!m_playing || m_score == 0 || m_len == 0) return;

    m_left_ms -= (int32_t)dt_ms;
    if(m_left_ms > 0) return;

    while(m_left_ms <= 0){
        if(m_i >= m_len){
            if(m_loop) m_i = 0;
            else { audio_music_stop(); return; }
        }
        audio_set_freq(m_score[m_i].freq);
        m_left_ms += (int32_t)m_score[m_i].dur_ms;
        m_i++;
    }
}

// =====================================================
// PLAYER BLOCCANTE (lasciato per SFX)
// =====================================================
static void busy_delay_ms(uint32_t ms){
    // Attenzione: dipende dalla frequenza CPU. Meglio SysTick/Timer in un mondo perfetto,
    // ma per SFX rapidi può bastare.
    volatile uint32_t n;
    while(ms--){
        for(n=0; n<100000; n++){
            __NOP();
        }
    }
}

void audio_play_tone(uint16_t freq_hz, uint16_t dur_ms){
    set_phase_step(freq_hz);
    busy_delay_ms(dur_ms);
    set_phase_step(0);
}

// =====================================================
// SFX
// =====================================================
void sfx_piece_drop(void){
    audio_set_volume(70);
    audio_play_tone(262, 50); // C4
}

void sfx_game_over(void){
    audio_set_volume(70);
    audio_play_tone(262, 120); // C4
    audio_play_tone(220, 120); // A3
    audio_play_tone(175, 180); // F3
}
