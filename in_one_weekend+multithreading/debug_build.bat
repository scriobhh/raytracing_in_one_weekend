@echo off

call "C:\VisualStudio\VC\Auxiliary\Build\vcvarsall.bat" x64

IF NOT EXIST .\build mkdir .\build
pushd .\build
cl -MTd -Od -Z7 -wd4477 ..\main.cpp
popd .\build

