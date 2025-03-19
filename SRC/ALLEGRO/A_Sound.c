#include "allegro.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/GENERAL/General.h"
#include "SRC/SYS/Def_inp.h"

struct SFX_file {
    char* filename;
    uint8_t looping;
};

struct SFX_file AllegroSFX_filenames[NUM_SFX];
SAMPLE* AllegroSFX_array;
id_type num_Allegrosfx = 0;

MIDI* the_music;
int Allegro_sfx_volume;
int Allegro_midi_volume;

int AllegroInitSounds()
{
    if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, "Allegro.cgf") != 0) {
        allegro_message("Error initialising sound system\n%s\n", allegro_error);
        return FALSE;
    }
    return TRUE;
}

int AllegroLoadMidi(char* filename)
{
    the_music = load_midi(filename);
    if (!the_music) {
        allegro_message("Error reading MIDI file '%s'\n", filename);
        return FALSE;
    }
    return TRUE;
}

void AllegroPlayMidi()
{
    play_midi(the_music, TRUE);
}

void AllegroStopMidi()
{
    stop_midi();
}

void AllegroSwitchMidiTrack(char* filename)
{
    // unload the old MIDI track and change to a new one
    stop_midi();
    destroy_midi(the_music);
    AllegroLoadMidi(filename);
    AllegroPlayMidi();
}

void AllegroChangeVolume(uint8_t volume_type, uint8_t change_type)
{
    int temp_sfx_vol;
    int temp_midi_vol;
    get_volume(&Allegro_sfx_volume, &Allegro_midi_volume);
    temp_sfx_vol = Allegro_sfx_volume;
    temp_midi_vol = Allegro_midi_volume;

    if (volume_type == SFX_VOLUME)
    {
        if (change_type == VOLUME_UP && temp_sfx_vol < 255)
            set_volume((temp_sfx_vol + 4), temp_midi_vol);
        else if (change_type == VOLUME_DOWN && temp_sfx_vol > 0)
            set_volume((temp_sfx_vol - 4), temp_midi_vol);
    }

    else if (volume_type == MIDI_VOLUME)
    {
        if (change_type == VOLUME_UP && temp_midi_vol < 255)
            set_volume(temp_sfx_vol, (temp_midi_vol + 4));
        else if (change_type == VOLUME_DOWN && temp_midi_vol > 0)
            set_volume(temp_sfx_vol, (temp_midi_vol - 4));
    }
}

void AllegroLoadBaseSFX()
{
    // loads a premade list of SFX files into memory
    int i;

    for (i = 0; i < num_Allegrosfx; i++)
    {
        SAMPLE* temp_spl = load_wav(AllegroSFX_filenames[i].filename);
        AllegroSFX_array[i] = *temp_spl;
    }
}

id_type AllegroLoadSFXFromFile(char* filename)
{
    // loads an individual SFX file into memory and returns its ID in the array
    id_type sfx_id = num_Allegrosfx;
    SAMPLE* temp_spl = load_wav(filename);

    AllegroSFX_array = realloc(AllegroSFX_array, (num_Allegrosfx + 1) * sizeof(SAMPLE));
    AllegroSFX_array[sfx_id] = *temp_spl;
    num_Allegrosfx++;

    return sfx_id;
}

// effect_id comes from the enum table SoundEffects
void AllegroPlaySFX(int effect_id)
{
    //if (SFX_on == TRUE)
        play_sample(&AllegroSFX_array[effect_id], Allegro_sfx_volume, 127, 1000, 0);
}

void AllegroDestroySFX()
{
    // deallocate memory for sound effects
    int i;
    for (i = 0; i < num_Allegrosfx; i++)
    {
        destroy_sample(&AllegroSFX_array[i]);
    }
}

void AllegroGenerateSFXFileTable()
{
    FILE* SFX_list_file;
    int i = 0;
    int looping;
    char filename[20];

    SFX_list_file = fopen("SFX/SFXLIST.txt", "r");

    if (SFX_list_file == NULL)
    {
        fclose(SFX_list_file);
        quitError("Unable to open the basic SFX list file!\n"
                  "Please check the file actually exists!\n");
        return;
    }

    do
    {
        fscanf(SFX_list_file, "%s %d", filename, &looping);
        AllegroSFX_filenames[i].filename = malloc(strlen(filename) + 1);
        strcpy(AllegroSFX_filenames[i].filename, filename);
        AllegroSFX_filenames[i].looping = looping;
        i++;
        num_Allegrosfx++;
    } while (fgetc(SFX_list_file) != EOF);
    
    fclose(SFX_list_file);
}

void AllegroSFXInit()
{
    /* Initialize array for sound effect filenames */
    printf("Generating base SFX file name table...\n");
    AllegroGenerateSFXFileTable();
    printf("OK!\n");
    printf("Loading base SFX files into memory...\n");
    AllegroSFX_array = malloc(num_Allegrosfx * sizeof(SAMPLE));
    AllegroLoadBaseSFX();
    printf("OK!\n");
    free(AllegroSFX_filenames);
}
