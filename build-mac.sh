set -e
mkdir -p ./build/

hotReload=0
run=0
gameProcessId=0

for arg in "$@"
do
    if [[ "$arg" = "--hot-reload" ]]; then
        hotReload=1
    fi

    if [[ "$arg" = "--run" ]]; then
        run=1
    fi
done

echo "=================================";
if [ $hotReload == 1 ]; then
    echo "Building game..."
    gcc ./src/game.cpp -o ./build/game -Wall -g -I ./include -L ./lib -dynamiclib -l SDL2-2.0.0
fi

echo "Building main..."
gcc ./src/main-mac.cpp -o ./build/main -Wall -g -I ./include -L ./lib -D HOT_RELOAD=$hotReload -l SDL2-2.0.0
echo "=================================";

if ! command -v fswatch &> /dev/null; then
    echo "fswatch is required for --hot-reload. More infos: https://github.com/emcrisostomo/fswatch"
    exit
fi

if [ $run == 1 ]; then
    # ./build/main
    ./build/main > /dev/null 2>&1 & 
    gameProcessId=$!
    # echo $gameProcessId
    # TODO: kill task when the game is closed
fi

if [ $hotReload == 1 ]; then
    fswatch -0 ./src | while read -d "" event; do
        # echo "$event changed!"
        echo "Rebuilding game..."
        gcc ./src/game.cpp -o ./build/game -Wall -g -I ./include -L ./lib -dynamiclib
    done
fi