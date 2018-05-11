@echo off
rem BUILD helper.
rem I don't want lots of copies of yotta-modules, and I can't
rem see an obvious way of getting yotta to compile just a 
rem particular cpp file; it seems hard-wired to main.cpp
rem
rem It seems quicker for me to solve it in a simple switch way
rem than to figure out yotta, so here it is.

rem Delete anything that potentially needs deleting from the
rem source folder. Leave microepi.h, as everything needs it.

del build\bbc-microbit-classic-gcc\source\*.hex >NUL
del source\serialno.h >NUL
del source\main.cpp >NUL

IF EXIST "projects\%1\main.cpp" (
  COPY "projects\%1\*.cpp" source >NUL
  COPY "projects\%1\*.h" source >NUL
  touch "source\main.cpp"
  yt build
) ELSE (
  ECHO projects\%1\main.cpp not found. Try....
  dir projects /b
)

  
