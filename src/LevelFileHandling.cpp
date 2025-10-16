#include <stdio.h>
#include <string.h>
#include "LevelFileHandling.h"
#include "GameStructs.h"

/* Level File Guidelines
 * -> One line == one record; do not split info for a given wall/sector across multiple lines.
 * -> sectors must have names proceeded by a colon, as below. This is used to set up walls' sector pointers.
 *    mySector: <fields here>
 * -> Sectors MUST precede walls in the file, as wall initialization depends on sectors already existing.
 * -> Wall and Sector sections should be denoted with the following headers:
 * [SECTORS]
 *  // sector data here
 * [WALLS]
 *  // wall data here
 * -> The order of walls and sectors within their respective sections is irrelevant.
 * -> Comments can be left in the file with '#' at the beginning of a line.
 *    # this is a comment
 * EXAMPLE FILE SYNTAX
 *
 * [SECTORS]
 * # FIELDS:
 * # bottomZ topZ
 * sector1: 0 30 
 * sector2: 0 50
 * sector3: 10 70
 * [WALLS]
 * # FIELDS:
 * # x1 y1 x2 y2 color sectorName
 * 10, 20, 10, 300, 5, sector1
 * -10, 20, -10, 300, 5, sector1
 * 10, 20, -10, 20, 5, sector1
 * 10, 300, 50, 350, 6, sector2
 *
 * */
// DEV NOTE: Consider following validations;
// 	* enforce max file line length (compare against internal maximum value)
// 	* enforce max file lines (i.e. no more than WALL_MAX + SECTOR_MAX lines, ~1333 by default)
// 	* error messages printed out for every violation, good for troubleshooting
void loadLevel(char filename[])
{
	FILE *fptr = fopen(filename, "r");
	char currentLine[MAX_LINE_CHARS];
	// tracks whether file contains proper header. Program aborts
	// gracefully if syntax is invalid
	bool hasSectorHeader = false;
	bool hasWallHeader = false;
	// look for [SECTORS] header
	while(fgets(currentLine, MAX_LINE_CHARS, fptr))
	{
		//printf("%s",currentLine);
		if(strcmp("[SECTORS]\n", currentLine) == 0) 
		{
			hasSectorHeader = true;
			break; 
		}
	}
	// read sectors until wall header is reached
	for (int i = 0; i < MAX_SECTORS; i++)
	{
		// continue until there are no new lines, or until [WALLS]
		// header is reached. whichever comes first
		if(fgets(currentLine, MAX_LINE_CHARS, fptr))
		{
			// skip comments
			if(currentLine[0] == '#') { continue; }
			printf("%s", currentLine); // debug
			if(strcmp("[WALLS]\n", currentLine) == 0) 
			{
				hasWallHeader = true;
				break; 
			}
			// populate new sector struct here
			//newSector = {};
			//Sectors[i] = newSector;
		}
		else
		{
			break;
		}
	}
	for (int i = 0; i < MAX_WALLS; i++)
	{
		// continue until there are no new lines, or until [WALLS]
		// header is reached. whichever comes first
		if(fgets(currentLine, MAX_LINE_CHARS, fptr))
		{
			// skip comments
			if(currentLine[0] == '#') { continue; }
			printf("%s", currentLine); // debug
			// populate wall structs here
		}
		else
		{
			break;
		}
	}
	if(!(hasSectorHeader && hasWallHeader))
	{
		printf("ERROR: Failed to open level. File lacks proper wall and sector headers.\n");
	}
	fclose(fptr);
}
