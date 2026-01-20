#include "audio.h"


#define E  1   // 1 tick
#define Q  2  // 2 tick
#define H  4  // 4 tick
#define P  0    // NIENTE pause


const NoteEvent tetris_theme[] = {
    // Frase A (sparata)
    {330, E}, {262, E}, {294, E}, {330, E},
    {392, Q},
    {330, E}, {262, E}, {294, E}, {330, E},
    {440, Q},

    // Frase B (risposta)
    {392, E}, {349, E}, {330, E}, {294, E},
    {262, Q},
    {294, E}, {330, E}, {349, E}, {392, E},
    {330, H},
};



void play_tetris_theme(void){
    audio_set_volume(70);
    audio_music_start(
        tetris_theme,
        sizeof(tetris_theme) / sizeof(tetris_theme[0]),
        1
    );
}

