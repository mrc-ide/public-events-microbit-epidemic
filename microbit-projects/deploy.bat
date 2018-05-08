@echo off
IF "%1"=="" (
  echo What drive to deploy to?
) else (
  copy build\bbc-microbit-classic-gcc\source\microbit-epidemic-combined.hex %1
)