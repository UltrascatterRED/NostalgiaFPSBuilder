#ifndef LEVEL_FILE_HANDLING_H
#define LEVEL_FILE_HANDLING_H

#define MAX_LINE_CHARS 50

bool seekLine(FILE* fptr, char linePattern[], char* lineBuffer, int lineSize);
void skipComments(FILE* fptr, char* lineBuffer, int lineSize);
void loadLevel(char filename[]);

// Total number of sectors and walls in the loaded level
extern int numSectors;
extern int numWalls;

#endif
