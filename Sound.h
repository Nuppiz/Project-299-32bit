#ifndef SOUND_H
#define SOUND_H

//id_type loadSFXFromFile(char* filename);
void playSFX(int effect_id);
void changeSFXVolume(int modifier);
void stopSFX();
void stopMusic();
void playMusic(char *music);
void changeMusicVolume(int modifier);
void initSounds();

#endif/* SOUND_H */
