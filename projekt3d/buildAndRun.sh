#!/bin/bash

g++ src/*.cpp -o checkers `pkg-config --cflags --libs openscenegraph`
./checkers