#!/bin/bash

errlog=gcc-errors.log
echo "## Error Log - $(timedatectl show -P RTCTimeUSec) ##" >> $errlog
gcc GameStructs.c Debug.c Render.c EngineMain.c LevelFileHandling.c -lGL -lGLU -lglut -lm -o NostalgiaFPS_Player 2>> $errlog && echo "Compilation Successful!"
exit 0
