#include "SRC/GAMEPLAY/Movecoll.h"
#include "SRC/GAMEPLAY/Game.h"
#include "SRC/GAMEPLAY/Def_game.h"
#include "SRC/GAMEPLAY/Def_ent.h"
#include "SRC/GAMEPLAY/Str_gplay.h"
#include "SRC/GENERAL/Common.h"
#include "SRC/SYS/Str_sys.h"

#include "Filech.h"
#include "LvlLoad.h"
#include "Def_lev.h"

/* Game save/load system */

extern Entity_t Entities[];
extern Item_t* Items;
extern System_t System;
extern Timer_t Timers;
extern GameData_t Game;
extern ActorTemplate_t ActorTemplates[];
extern int actortemplate_count;
extern char levelname_global[];

void saveGameState(char* foldername)
{
    FILE* save_file;
    char savefilepath[50] = "SAVES/";

    strcat(savefilepath, foldername);
    if (!checkDirectoryExists(savefilepath))
    {
        createDirectory(savefilepath);
    }
    strcat(savefilepath, "CURSTATE.SAV");

    save_file = fopen(savefilepath, "wb");
    if (!save_file)
    {
        perror("fopen");
    }
    fwrite(&Game.current_level_name, LEVEL_NAME_MAX, 1, save_file);
    fwrite(&PLAYER_ACTOR.health, 2, 1, save_file);
    fwrite(&PLAYER_ACTOR.position.x, 8, 1, save_file);
    fwrite(&PLAYER_ACTOR.position.y, 8, 1, save_file);
    fwrite(&PLAYER_ACTOR.primary_weapon_id, 2, 1, save_file);
    fwrite(&System, sizeof(System_t), 1, save_file);
    fwrite(&Timers, sizeof(Timer_t), 1, save_file);
    fclose(save_file);
}

void saveLevelState(char* foldername, char* levelname)
{
    FILE* save_file;
    char savefilepath[50] = "SAVES/";

    strcat(savefilepath, foldername);
    if (!checkDirectoryExists(savefilepath))
    {
        createDirectory(savefilepath);
    }

    strcat(levelname, ".SAV");
    strcat(savefilepath, levelname);

    save_file = fopen(savefilepath, "wb");
    if (!save_file)
    {
        printf("%s", levelname);
        perror("fopen");
    }
    fwrite(&Game, sizeof(GameData_t), 1, save_file);
    fwrite(Game.Actors, sizeof(Actor_t), Game.actor_capacity, save_file);
    fwrite(Game.ActorsById, sizeof(id_type), Game.id_capacity, save_file);
    fwrite(ActorTemplates, sizeof(ActorTemplate_t), actortemplate_count, save_file);
    fwrite(Entities, sizeof(Entity_t), MAX_ENTITIES, save_file);
    fwrite(Items, sizeof(Item_t), Game.item_capacity, save_file);
    System.paused = FALSE;
    fclose(save_file);
}

void loadGameState(char* foldername)
{
    FILE* state_file;
    char savefilepath[50] = "SAVES/";

    strcat(savefilepath, foldername);
    strcat(savefilepath, "CURSTATE.SAV");

    if (checkFileExists(savefilepath))
    {
        state_file = fopen(savefilepath, "rb");
        fseek(state_file, 0x0F, SEEK_SET);
        fread(&PLAYER_ACTOR.health, 2, 1, state_file);
        fread(&PLAYER_ACTOR.position.x, 8, 1, state_file);
        fread(&PLAYER_ACTOR.position.y, 8, 1, state_file);
        fread(&PLAYER_ACTOR.primary_weapon_id, 2, 1, state_file);
        updateGridLoc(&PLAYER_ACTOR);
        // just in case the player's location in the save file IS on a portal (shouldn't be if level is made correctly)
        if (getEntityTypeAt(PLAYER_ACTOR.grid_loc) == ENT_PORTAL)
        {
            PLAYER_ACTOR.position = forceMove(PLAYER_ACTOR.position);
            updateGridLoc(&PLAYER_ACTOR);
        }
        fread(&System, sizeof(System_t), 1, state_file);
        fread(&Timers, sizeof(Timer_t), 1, state_file);
        fclose(state_file);
    }
    testInitPlayerAnim();
}

void loadLevelState(char* foldername, char* savename)
{
    FILE* save_file;
    char savefilepath[50] = "SAVES/";

    strcat(savefilepath, foldername);
    strcat(savefilepath, savename);

    save_file = fopen(savefilepath, "rb");
    if (!save_file)
    {
        printf("%s", savename);
        perror("fopen");
    }
    fseek(save_file, 0x28, SEEK_SET);
    fread(&Game.actor_count, 2, 1, save_file);
    fread(&Game.actor_capacity, 2, 1, save_file);
    fread(&Game.id_capacity, 2, 1, save_file);
    initGameData(Game.actor_capacity, Game.id_capacity);
    fread(&Game.item_count, 1, 1, save_file);
    fread(&Game.item_capacity, 1, 1, save_file);
    Items = malloc(Game.item_capacity * sizeof(Item_t));
    fread(&Game.player_id, 2, 1, save_file);
    fseek(save_file, 0x34, SEEK_SET);
    fread(Game.Actors, sizeof(Actor_t), Game.actor_capacity, save_file);
    fread(Game.ActorsById, sizeof(id_type), Game.id_capacity, save_file);
    fread(ActorTemplates, sizeof(ActorTemplate_t), actortemplate_count, save_file);
    fread(Entities, sizeof(Entity_t), MAX_ENTITIES, save_file);
    fread(Items, sizeof(Item_t), Game.item_capacity, save_file);
    fclose(save_file);
    setEntityTilemap();
}

void loadAfterDeath(char* currentlevel)
{
    char loadsavename[30] = {'\0'};
    char savepath[45] = "SAVES/CURRENT/";

    strncpy(loadsavename, currentlevel, (strlen(currentlevel) - 4)); // drop the level filename ending
    strcat(loadsavename, ".SAV"); // add save filename ending
    strcat(savepath, loadsavename); // construct folder path for file existence check

    if (checkFileExists(savepath)) // if savefile exists, load save
    {
        levelLoader(currentlevel, LOAD_SAVED_LEVEL);
        loadLevelState("CURRENT/", loadsavename);
        loadGameState("CURRENT/");
    }
    else // else load everything from level file
        levelLoader(currentlevel, LOAD_NEW_LEVEL);
}

// check level name from save file, return FALSE if CURSTATE cannot be opened
int checkLevelFromSave(char* foldername)
{
    FILE* save_file;
    char savefilepath[50] = "SAVES/";
    int i;

    strcat(savefilepath, foldername);
    strcat(savefilepath, "CURSTATE.SAV");

    if (checkFileExists(savefilepath))
    {
        save_file = fopen(savefilepath, "rb");
        for (i = 0; i < LEVEL_NAME_MAX; i++)
        {
            fread(&levelname_global[i], 1, 1, save_file);
        }
        fclose(save_file);
        return TRUE;
    }
    return FALSE;
}

void saveGame(char* foldername)
{
    char savepath[25] = "SAVES/"; // needed for the copy function
    char savefilename[15] = {'\0'};

    strcat(foldername, "/"); // add slash to the save subfolder name
    strcat(savepath, foldername);

    if (!checkDirectoryExists(savepath)) // if folder doesn't exist, create it
        createDirectory(savepath);

    copyAllFolderToFolder("SAVES/CURRENT/", savepath);
    strncpy(savefilename, Game.current_level_name, (strlen(Game.current_level_name) - 4)); // drop the level filename ending
    saveLevelState(foldername, savefilename);
    saveGameState(foldername);
}

int loadGame(char* foldername)
{
    char loadname[30] = {'\0'};
    char savepath[45] = "SAVES/";
    char savefolder[45] = "SAVES/";
    strcat(foldername, "/"); // add subfolder slash
    strcat(savepath, foldername); // construct full save folder path
    strcat(savefolder, foldername); // separate copy for the file copying function
    if (checkLevelFromSave(foldername)) // if CURSTATE is opened successfully, continue function
    {
        strncpy(loadname, levelname_global, (strlen(levelname_global) - 4)); // drop the level filename ending
        strcat(loadname, ".SAV"); // add save filename ending
        strcat(savepath, loadname); // construct full save file path

        if (checkFileExists(savepath)) // if savefile exists, load save, else do nothing
        {
            levelLoader(levelname_global, LOAD_SAVED_LEVEL);
            loadLevelState(foldername, loadname);
            loadGameState(foldername);
            deleteDirectoryContents("SAVES/CURRENT/"); // empty CURRENT folder
            copyAllFolderToFolder(savefolder, "SAVES/CURRENT/"); // copy all contents to the CURRENT save folder
            levelname_global[0] = '\0'; // reset levelname
            return TRUE;
        }
    }
    levelname_global[0] = '\0'; // reset levelname
    return FALSE;
}

void quickLoad()
{
    loadGame("QUICK");
}

void quickSave()
{
    saveGame("QUICK");
}
