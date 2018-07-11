# file: build.sh
swig -python -Isrc game_controls.i

gcc -Isrc -fPIC $(pkg-config --cflags --libs python3) -c game_controls.c game_controls_wrap.c
gcc -shared  -fPIC -o _game_controls.so game_controls.o game_controls_wrap.o

python3 -c "import game_controls" # test
