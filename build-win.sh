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

echo "============================================";
if [ $hotReload == 1 ]; then
    ./build-game-win.bat
fi

echo "Building main..."
gcc ./src/main-win.cpp -o ./build/main -Wall -g -I ./include/win64/** -L ./lib/win64/** -lmingw32 -lSDL2main -lSDL2 -mwindows -D HOT_RELOAD=$hotReload
echo "============================================";

if [ $run == 1 ]; then
    ./build/main
fi