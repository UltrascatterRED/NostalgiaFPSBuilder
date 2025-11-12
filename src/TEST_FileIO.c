#include <stdio.h>
#include "GameStructs.h"
#include "LevelFileHandling.h"

/* TEST PROGRAM FOR LEVEL FILE I/O OPERATIONS
*   This file sources LevelFileHandling.h along with minimum
*   dependencies to conduct a sort of "unit test", the unit
*   being the whole codefile.
*/
int main()
{
    char fileName[] = "TestLevelFile.txt";
    loadLevel(fileName);
    printf("numSectors = %d, numWalls = %d\n", numSectors, numWalls);
    printf("SECTORS\n");
    for(int i = 0; i < numSectors; i++)
    {
        printf("%p: { bottomZ=%d,"
        "topZ=%d,"
        "centerX=%d,"
        "centerY=%d,"
        "numChildren=%d,"
        "playerProximity=%d,"
        "hasCaps=%d }\n",
        &Sectors[i],
        Sectors[i].bottomZ,
        Sectors[i].topZ,
        Sectors[i].centerX,
        Sectors[i].centerY,
        Sectors[i].numChildren,
        Sectors[i].playerProximity,
        Sectors[i].hasCaps);
    }
    printf("WALLS\n");
    for(int i = 0; i < numWalls; i++)
    {
        printf("{ x1=%d, y1=%d, x2=%d, y2=%d, color=%d, parent=%p }\n",
        Walls[i].x1,
        Walls[i].y1,
        Walls[i].x2,
        Walls[i].y2,
        Walls[i].color,
        Walls[i].parentSector);
    }
    printf("LEVEL I/O TEST COMPLETE\n");
    return 0;
}
