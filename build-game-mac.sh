set -e

echo "Building game..."
gcc ./src/game.cpp -o ./build/game.o \
    -Wall \
    -g \
    -I ./include/mac/** \
    -L ./lib/mac/** \
    -l SDL2 -l SDL2_image \
    -dynamiclib