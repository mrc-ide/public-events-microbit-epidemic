@echo off
javac *.java
echo Manifest-Version: 1.0>>MANIFEST.MF
echo Created-By: 1.8.0_172 (Oracle Corporation)>>MANIFEST.MF
echo Main-Class: MakeStickers>>MANIFEST.MF
jar cvfm MakeStickers.jar MANIFEST.MF *.class >nul
del *.class >nul
del MANIFEST.MF >nul
