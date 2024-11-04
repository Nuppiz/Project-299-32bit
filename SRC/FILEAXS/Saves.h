#ifndef SAVES_H
#define SAVES_H

void saveGameState(char* foldername);
void saveLevelState(char* foldername, char* levelname);
void loadGameState(char* foldername);
void loadLevelState(char* foldername, char* savename);
void loadAfterDeath(char* currentlevel);
int checkLevelFromSave(char* foldername);
void saveGame(char* foldername);
int loadGame(char* foldername);
void quickLoad();
void quickSave();

#endif /* SAVES_H */
