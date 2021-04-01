set -e
mkdir -p ./build/

hotReload=0
run=0

for arg in "$@"; do
    if [[ "$arg" = "--hot-reload" ]]; then
        hotReload=1
    fi

    if [[ "$arg" = "--run" ]]; then
        run=1
    fi
done

echo "=================================";
if [ $hotReload == 1 ]; then
    ./mac_build_game.sh
fi

echo "Building tycoon (mac)..."
gcc ./src/mac_game.cpp -o ./build/tycoon \
    -Wall \
    -g \
    -I ./include/mac/** \
    -L ./lib/mac/** \
    -l SDL2 -l SDL2_image \
    -D HOT_RELOAD=$hotReload
echo "=================================";

if [ $run == 1 ]; then
    ./build/tycoon
fi