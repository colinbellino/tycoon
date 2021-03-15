@echo off

echo Building game...
gcc ./src/game.cpp -o ./build/game.dll -Wall -g -I ./include -L ./lib -lmingw32 -lSDL2main -lSDL2 -mwindows -shared