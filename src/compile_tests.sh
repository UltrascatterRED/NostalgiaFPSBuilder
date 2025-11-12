#!/bin/bash

gcc TEST_FileIO.c LevelFileHandling.c GameStructs.c -lm -o TEST_FileIO

gcc TEST_Render.c GameStructs.c LevelFileHandling.c Render.c -lGL -lGLU -lglut -lm -o TEST_Render

exit 0
