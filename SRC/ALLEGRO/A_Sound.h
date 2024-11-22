#ifndef A_SOUND_H
#define A_SOUND_H

int AllegroInitSounds();
int AllegroLoadMidi(char* filename);
void AllegroPlayMidi();
void AllegroSwitchMidiTrack(char* filename);
void AllegroChangeVolume(uint8_t volume_type, uint8_t change_type);
void AllegroPlaySFX(int effect_id);
void AllegroSFXInit();

#endif /* A_SOUND_H */
