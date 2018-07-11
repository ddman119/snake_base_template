# file: build.sh
swig -python -c++ -Isrc game_controls.i

g++ -Isrc -fPIC $(pkg-config --cflags --libs python3) -c game_controls.cpp game_controls_wrap.cxx
g++ -shared  -fPIC -o _game_controls.so game_controls.o game_controls_wrap.o

python3 -c "import game_controls" # test
