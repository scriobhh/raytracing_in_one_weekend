@echo off

call "C:\VisualStudio\VC\Auxiliary\Build\vcvarsall.bat" x64

IF NOT EXIST .\build mkdir .\build
pushd .\build
cl -MT -O2 -wd4477 -wd4530 ..\main.cpp
popd .\build

