#include <stdio.h>
#include "Render.h"
#include "GameStructs.h"
#include "LevelFileHandling.h"

int main()
{
    char fileName[] = "TestLevelFile.txt";
    loadLevel(fileName);
    drawView();
    return 0;
}
