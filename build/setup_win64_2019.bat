@echo off

mkdir g4gBuild
cd g4gBuild
cmake -G "Visual Studio 16 2019" ../../src/Project2
cd ..
