#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "LevelFileHandling.h"
#include "GameStructs.h"

/* Level File Guidelines
 * -> One line == one record; do not split info for a given wall/sector across 
 *    multiple lines.
 * -> Comments can be left in the file with '#' at the beginning of a line.
 *    -> Comments CANNOT be inline; they must always have a line to themselves.
 *    # this is a comment
 * -> Each sector must be grouped with its child walls like so:
 *    [SECTOR]
 *    0 40
 *    [WALLS]
 *    # these walls are children of the above sector
 *    -20 20 -20 300 5
 *    20 20 20 300 5
 *    -20 20 20 20 5
 *    -20 300 20 300 5
 *    [END WALLS]
 * EXAMPLE FILE SYNTAX
 * --------------------------------------
 * [SECTOR]
 * # FIELDS:
 * # bottomZ topZ
 * 0 30 
 * [WALLS]
 * # FIELDS:
 * # x1 y1 x2 y2 color 
 * 10, 20, 10, 300, 5
 * -10, 20, -10, 300, 5
 * 10, 20, -10, 20, 5
 * [END WALLS]
 * [SECTOR]
 * 0 50
 * [WALLS]
 * 10, 300, 50, 350, 6
 * [END WALLS]
 * --------------------------------------
 * */
// Given a file pointer and a pattern, find first occurrence
// of a matching line in the file pointed to by fptr. Returns
// true on a match, false otherwise. When a match is found,
// leaves fptr at that position and writes content of the
// line to the provided lineBuffer of lineSize bytes/chars.
// Otherwise, lineBuffer will simply contain the last line
// of the file.


bool seekLine(FILE* fptr, char linePattern[], char* lineBuffer, int lineSize)
{
	long ptrInitPos = ftell(fptr);
	while(fgets(lineBuffer, lineSize, fptr))
	{
		if(strcmp(lineBuffer, linePattern) == 0) { return true; }
	}
	// reset file pointer position
	long ptrPos = ftell(fptr);
	fseek(fptr, ptrInitPos, 0);
	return false;
}
//  conditionally moves fptr forward until a non-commented
//  line is encountered. Writes the line stopped on to
//  the provided lineBuffer of lineSize bytes/chars.
void skipComments(FILE* fptr, char* lineBuffer, int lineSize)
{
    do
	{
		if(lineBuffer[0] == '#')
		{
			//printf("%c comment skipped\n", lineBuffer[0]);
			continue;
		}
		else
		{
			//printf("Stopped on this line:\n\"%s\"\n", lineBuffer);
			break;
		}
	}
	while(fgets(lineBuffer, lineSize, fptr));
}
// DEV NOTE: Consider following validations;
// 	* enforce max file line length (compare against internal maximum value)
// 	* enforce max file lines (i.e. no more than WALL_MAX + SECTOR_MAX lines, 
//    ~1333 by default)
// 	* error messages printed out for every violation, good for troubleshooting
// DEV NOTE: Decomposition of this function advised

int numSectors = 0;
int numWalls = 0;
void loadLevel(char filename[])
{
    //printf("DEBUG: Loading level\n");
	FILE *fptr = fopen(filename, "r");
	char currentLine[MAX_LINE_CHARS];
	// Tracks whether file contains proper headers. 
	// Every SECTOR header needs a corresponding WALL
	// header and vice versa.
	// Program aborts gracefully if syntax is invalid
	bool hasSectorHeader = false;
	bool hasWallHeader = false;
    char sectorHeader[] = "[SECTOR]\n";
    char wallHeader[] = "[WALLS]\n";
    char wallFooter[] = "[END WALLS]\n";
	while(fgets(currentLine, MAX_LINE_CHARS, fptr))
	{
		//printf("%s",currentLine);
		if(strcmp(sectorHeader, currentLine) == 0) 
		{
			hasSectorHeader = true;
            //numSectors++;
		}
		if(strcmp(wallHeader, currentLine) == 0)
		{
			hasWallHeader = true;
		}
		// when true, this confirms a syntactically correct sector/wall 
        // header pair
		if(hasSectorHeader && hasWallHeader)
		{
			hasSectorHeader = false;
			hasWallHeader = false;
		}
    }
	// when true, sector/wall headers are syntactically incorrect, and level file
	// cannot be read
	if(hasSectorHeader != hasWallHeader)
	{
		printf("ERROR: Incorrect Syntax - Level file cannot be read."
        "Verify sector/wall headers are correct.\n");
		return;
	}
    //printf("DEBUG: Syntax check passed\n");
	// temp buffers for sector fields
	int bottomZ;
	int topZ;
	int centerX;
	int centerY;
    int numChildren;
    // reset file pointer to start of file
    fseek(fptr, 0, SEEK_SET);
	while(fgets(currentLine, MAX_LINE_CHARS, fptr))
	{
        skipComments(fptr, currentLine, MAX_LINE_CHARS);
		if(strcmp(sectorHeader, currentLine) == 0) 
		{
			// populate new sector struct here
            numSectors++;
            fgets(currentLine, MAX_LINE_CHARS, fptr);
            skipComments(fptr, currentLine, MAX_LINE_CHARS);
            sector secBuffer;
			// 1) split fields on whitespace
			char *tokenPtr = strtok(currentLine, " ");
            //printf("DEBUG: token ptr initialized\n");
			// 2) parse to numbers
			bottomZ = atoi(tokenPtr);
			tokenPtr = strtok(NULL, " "); //advance token pointer
			topZ = atoi(tokenPtr);
			// 3) remaining members must be calculated from walls
            int numChildWalls = 0;
			//populate wall structs
		    if(seekLine(fptr, wallHeader, currentLine, MAX_LINE_CHARS))
            {
                int x1, y1, x2, y2, color;
                // used to average child wall points, thus determining
                // the parent sector's center coords 
                float xSum, ySum;
                fgets(currentLine, MAX_LINE_CHARS, fptr);
                skipComments(fptr, currentLine, MAX_LINE_CHARS);
                numChildWalls = 0;
                while(strcmp(wallFooter, currentLine) != 0)
                {
                    numWalls++;
                    numChildWalls++;

                    skipComments(fptr, currentLine, MAX_LINE_CHARS);
                    tokenPtr = strtok(currentLine, " ");
                    //iterate over tokens and set wall struct members
                    //TODO: validate ints here via new function before
                    //      assigning atoi() calls
                    
                    x1 = atoi(tokenPtr);
                    xSum += x1;

                    tokenPtr = strtok(NULL, " ");
                    y1 = atoi(tokenPtr);
                    ySum += y1;

                    tokenPtr = strtok(NULL, " ");
                    x2 = atoi(tokenPtr);
                    xSum += x2;

                    tokenPtr = strtok(NULL, " ");
                    y2 = atoi(tokenPtr);
                    ySum += y2;

                    tokenPtr = strtok(NULL, " ");
                    color = atoi(tokenPtr);

                    wall wallBuffer = {x1, y1, x2, y2, color, &Sectors[numSectors - 1]};
                    Walls[numWalls - 1] = wallBuffer;
                    fgets(currentLine, MAX_LINE_CHARS, fptr);
                }
                // 4) calculate centerX, centerY
                centerX = (int) roundf(xSum / (float) numChildWalls);
                centerY = (int) roundf(ySum / (float) numChildWalls);
            }
            //printf("  DEBUG: Child Wall Parse Finished\n");

			// 5) write to struct
			// 0 is initial value of playerProximity, which is constantly 
            // recalculated at runtime
			// "false" is default value of hasCaps (see struct definition 
            // in GameStructs.h)
			sector newSector = { bottomZ, topZ, centerX, centerY, numChildWalls, 0, false };
			Sectors[numSectors - 1] = newSector;
            //numSectors++;
		}
        else
        {
            //consider printing line number of failure as well
            printf("ERROR: Bad syntax in level file. Aborting...\n");
            break;
        }
        //printf("DEBUG: Sector Parse Finished\n");
	}
	fclose(fptr);
}
