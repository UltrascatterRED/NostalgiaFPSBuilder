#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include "GameStructs.h"
#include "Debug.h"
#include "Render.h"

// Counts the number of times displayFrame() has executed. 
// Used to display FPS at regular intervals.
int framesDrawn = 0; 
// Number of milliseconds between FPS printouts
unsigned int fpsDisplayRate = 1000; // default: 1000 
// timestamp in milliseconds since last print
int lastPrint = 0;

// Prints out the last recorded average FPS to the terminal
// Must be called every frame for accurate results.
void printFPS()
{
	framesDrawn++;
    int elapsedTime = glutGet(GLUT_ELAPSED_TIME);
    if(elapsedTime - lastPrint >= fpsDisplayRate)
    {
        // divide by const 1000 to convert ms to seconds
        int FPS = (int) round((float)framesDrawn/((float)fpsDisplayRate/1000)); 
        printf("%d FPS\n", FPS);
        framesDrawn = 0;
        lastPrint = elapsedTime;
    }
}

// DEBUG; Displays a "palette" showing all supported colors and animated frames
int tick = 0;
void drawTest()
{
	int swHalf = SCREEN_WIDTH/2;
	int shHalf = SCREEN_HEIGHT/2;
	int c = 0; // color ID
	for(int y = 0; y < shHalf; y++)
	{
		for(int x = 0; x < swHalf; x++)
		{
			drawPixel(x, y, c); 
			c += 1;
			if(c > 8) { c = 0; }
		}
	}
	//frame rate
	tick += 1; 
	if(tick>20) { tick = 0; } 
	drawPixel(swHalf, shHalf+tick, 6); 
}

void printSectorInfo(sector* loc)
{
    printf("----------------------------------------\n"
    "INFO FOR SECTOR AT [%p]:\n"
    "bottomZ = %d,\n"
    "topZ = %d,\n"
    "centerX = %d,\n"
    "centerY = %d,\n"
    "numChildren = %d,\n"
    "playerProximity = %d,\n"
    "hasCaps = %d\n"
    "----------------------------------------\n",
    loc, 
    loc->bottomZ, 
    loc->topZ, 
    loc->centerX, 
    loc->centerY,
    loc->numChildren,
    loc->playerProximity,
    loc->hasCaps);
}

void printAllSectorsInfo()
{
    for(int i = 0; i < MAX_SECTORS; i++)
    {
        printf("(INDEX %d) ", i);
        printSectorInfo(&Sectors[i]);
    }
}

void printWallInfo(wall* loc)
{
    printf("------------------------------------\n"
    "INFO FOR WALL AT [%p]:\n"
    "x1 = %d,\n"
    "y1 = %d,\n"
    "x2 = %d,\n"
    "y2 = %d,\n"
    "color = %d,\n"
    "sector = %p\n"
    "------------------------------------\n",
    loc,
    loc->x1,
    loc->y1,
    loc->x2,
    loc->y2,
    loc->color,
    loc->parentSector);
}

void printAllWallsInfo()
{
    for(int i = 0; i < MAX_WALLS; i++)
    {
        printf("(INDEX %d) ");
        printWallInfo(&Walls[i]);
    }
}
