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
    int numChildren = 0;
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
			//populate wall structs
		    if(seekLine(fptr, wallHeader, currentLine, MAX_LINE_CHARS))
            {
                int x1, y1, x2, y2, centX, centY, pProx, color;
                // used to average child wall points, thus determining
                // the parent sector's center coords 
                float xSum = 0;
                float ySum = 0;
                fgets(currentLine, MAX_LINE_CHARS, fptr);
                skipComments(fptr, currentLine, MAX_LINE_CHARS);
                numChildren = 0;
                while(strcmp(wallFooter, currentLine) != 0)
                {
                    numWalls++;
                    numChildren++;

                    skipComments(fptr, currentLine, MAX_LINE_CHARS);
                    tokenPtr = strtok(currentLine, " ");
                    //iterate over tokens and set wall struct members
                    //TODO: validate ints here via new function before
                    //      assigning atoi() calls
                    
                    //printf("DEBUG: parsing token \"%s\" -> x1\n", tokenPtr);
                    x1 = atoi(tokenPtr);
                    //printf("x1 = %d\n", x1);
                    xSum += x1;

                    tokenPtr = strtok(NULL, " ");
                    //printf("DEBUG: parsing token \"%s\" -> y1\n", tokenPtr);
                    y1 = atoi(tokenPtr);
                    //printf("y1 = %d\n", y1);
                    ySum += y1;

                    tokenPtr = strtok(NULL, " ");
                    //printf("DEBUG: parsing token \"%s\" -> x2\n", tokenPtr);
                    x2 = atoi(tokenPtr);
                    //printf("x2 = %d\n", x2);
                    xSum += x2;

                    tokenPtr = strtok(NULL, " ");
                    //printf("DEBUG: parsing token \"%s\" -> y2\n", tokenPtr);
                    y2 = atoi(tokenPtr);
                    //printf("y2 = %d\n", y2);
                    ySum += y2;

                    tokenPtr = strtok(NULL, " ");
                    //printf("DEBUG: parsing token \"%s\" -> color\n", tokenPtr);
                    color = atoi(tokenPtr);
                    //printf("color = %d\n\n", color);

                    // calculate center point of wall; float cast for better accuracy
                    centX = (int) roundf(((float)x1 + (float)x2)/2);
                    centY = (int) roundf(((float)y1 + (float)y2)/2);
                    //printf("Wall %d: centX = %d, centY = %d\n", numWalls, centX, centY);
                    // playerProximity initialized to 0; it is constantly recalculated at runtime
                    wall wallBuffer = {x1, y1, x2, y2, centX, centY, 0, color, &Sectors[numSectors - 1]};
                    Walls[numWalls - 1] = wallBuffer;
                    fgets(currentLine, MAX_LINE_CHARS, fptr);
                }
                // 4) calculate centerX, centerY
                // divide by numChildren*2 to account for redundant coord summation
                centerX = (int) roundf(xSum / (float) (numChildren*2));
                centerY = (int) roundf(ySum / (float) (numChildren*2));
                // debug
                //printf("Sector %d: centerX = %f/%d = %d, centerY = %f/%d = %d\n", 
                //numSectors-1, xSum, numChildren, centerX, 
                //ySum, numChildren, centerY);
            }
            //printf("  DEBUG: Child Wall Parse Finished\n");

			// 5) write to struct
			// 0 is initial value of playerProximity, which is constantly 
            // recalculated at runtime
			// "false" is default value of hasCaps (see struct definition 
            // in GameStructs.h)
			sector newSector = { bottomZ, topZ, centerX, centerY, numChildren, 0, false };
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
