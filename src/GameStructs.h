#ifndef GAMESTRUCTS_H
#define GAMESTRUCTS_H

#define MAX_WALLS 1000
#define MAX_SECTORS 333

typedef struct
{
	int frame1, frame2;
}bufferTime;
// stores keyboard input state
typedef struct
{
	// move forward, move back, strafe left, strafe right, look left, look right, fly up, fly down
	int moveF, moveB, strafeL, strafeR, lookL, lookR, flyU, flyD;
}keyState;
// stores pre-calculated sine and cosine values to avoid calculating at 
// runtime 
typedef struct
{
	float sin[360];
	float cos[360];
}trigVals;
// player state information
typedef struct
{
	int x, y, z; // 3D position; z is up/down
	int angle; // angle of player's looking direction
}player;

// sectors denote groups of 1 or more walls. When there are 3 or more connected
// walls, sectors are continuous 3D prisms, where the walls form a "tube" and the bottom
// and top faces are filled. Though not required, it is recommended to only group
// CONNECTED walls in the same sector.
typedef struct
{
	//int firstWallIdx; // global wall array index of first wall in sector
	//int lastWallIdx; // global wall array index of last wall in sector
	int bottomZ; // z location of bottom edge
	int topZ; // z location of top edge
	int centerX; // x coord of sector's center
	int centerY; // y coord of sector's center
    int numChildren; // number of child walls belonging to this sector
	int playerProximity; // used to determine onscreen draw order of sectors
	// sector must have AT LEAST 2 walls to draw valid caps
	bool hasCaps; // used to determine whether to draw floors/ceilings (aka sector "caps")
    int capColor; // same color codes as walls
}sector;

// wall-specific information (all other info handled by struct sector)
// All walls must belong to a sector.
typedef struct
{
	int x1;
	int y1;
	int x2;
	int y2;
    int centerX;
    int centerY;
    int playerProximity;
	int color;	// wall color lookup code
	sector* parentSector; // pointer to parent sector
}wall;

void init(); // initializes static structs to necessary values
extern bufferTime Bft;
extern keyState KeyState;
extern trigVals TrigVals;
extern player Player;
extern wall Walls[MAX_WALLS];
extern sector Sectors[MAX_SECTORS];

#endif
