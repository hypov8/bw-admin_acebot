@echo off

del include\strings.h
del src\strings.c

crypt.exe src\strings.in.c

move strings.h include
move strings.c src
