#!/bin/sh

includes="-Iinclude"
warnings="-Wno-writable-strings -Wno-deprecated-declarations -Wno-format-security"
libs="-luser32.lib -lopengl32.lib -lgdi32.lib"

timestamp=$(date +%s)

clang $includes -g src/engine.cpp -o bin/engine.exe $libs $warnings