#!/bin/bash

errlog=gcc-errors.log
echo "## Error Log - $(timedatectl show -P RTCTimeUSec) ##" >> $errlog
g++ GameStructs.cpp Debug.cpp Render.cpp EngineMain.cpp LevelFileHandling.cpp -lGL -lGLU -lglut -lm -o NostalgiaFPS_Player 2>> $errlog && echo "Compilation Successful!"
exit 0
