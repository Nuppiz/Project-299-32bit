#include "allegro.h"

MIDI *the_music;

int playThatFunkyMusic()
{
    if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0) {
        allegro_message("Error initialising sound system\n%s\n", allegro_error);
        
        return 1;
    }
    the_music = load_midi("MUSIC\\E1M1.MID");
    if (!the_music) {
        allegro_message("Error reading MIDI file '%s'\n", "MUSIC\\E1M1.MID");
        delay(500);
        return 1;
    }
    play_midi(the_music, TRUE);

    return FALSE;
}