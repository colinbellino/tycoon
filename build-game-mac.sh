set -e

echo "Building game..."
gcc ./src/game.cpp -o ./build/game.o -Wall -g -I ./include/mac/** -L ./lib/mac/** -dynamiclib -l SDL2-2.0.0