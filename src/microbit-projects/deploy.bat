@echo off
IF "%1"=="" (
  echo What drive to deploy to?
) else (
  :WAITFOR
  TIMEOUT 1
  IF NOT EXIST %1\MICROBIT.HTM GOTO WAITFOR
  copy build\bbc-microbit-classic-gcc\source\microbit-epidemic-combined.hex %1
)