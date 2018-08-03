#! /bin/bash

g++ -o snake global.h client.h client.cpp server.h server.cpp backend.cpp -lpthread

mv ./snake ../bin/snake

cp ./snake.py ../bin/snake.py
