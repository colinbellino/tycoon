@echo off

echo Building game...
gcc ./src/game.cpp -o ./build/game.dll -Wall -g -I ./include/win64/** -L ./lib/win64/** -lmingw32 -lSDL2main -lSDL2 -mwindows -shared