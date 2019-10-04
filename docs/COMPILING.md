# public-events-microbit-epdemic: Compiling Guide

Here, we describe how to create an environment on windows, 
for compiling our code into binaries to copy to the micro:bit.
In due course, I'll try this on other platforms, and perhaps
make a docker container which will make life easier.

## Compiling for micro:bit

### Compiler Tools on Windows

Here's how I do offline compilation of C code to Microbit, on Windows.
All of the below is based upon the manual instructions 
[here](http://docs.yottabuild.org/#manual-windows-installation); the
automatic installer didn't work for me - yotta.exe never came into
existence. But the below worked ok.

* Install the latest Python 2.7. The build tool is written in python, and 
support for Python 3 is stated as _experimental_. I installed 
[Python 2.7.15 64-bit](https://www.python.org/ftp/python/2.7.15/python-2.7.15.msi). I 
let it install to `C:\Python27`.

* There was no "Add to Path" option in the python installer strangely. So, 
add `C:\Python27` and `C:\Python27\Scripts` to the system path in Control Panel, 
System, Advanced system settings, Environment Variables, Path.

* New Command window, and run `Python` to check you get 2.7.15. (Multiple
python versions and packages on the same machine is a specific kind of nightmare).

* `quit()` python, and upgrade pip with `python -m pip install --upgrade pip`

* Install the latest 64-bit CMake, currently 
[here](https://github.com/Kitware/CMake/releases/download/v3.13.2/cmake-3.13.2-win64-x64.msi).
It has an option to add to the system path, but this didn't work, so I manually added
`C:/CMake/bin` to the system path, as earlier.

* Install [Ninja](https://github.com/ninja-build/ninja/releases)
- I took the latest version, 1.8.2. It's a single executable 
file, so I cheated a bit, and put it in `C:/Python27/Scripts` 
which is already in the path, since its only one python-related file.

* Install the gcc cross-compiler (gcc-arm-none-eabi). The 
Yotta notes talk about an old version 4.9. I've found 2017-q4 works, 2018-q4 doesn't 
(e.g. arm-none-eabi-objcopy: microbit-samples.hex 64-bit address 0x4b4fa300018000 out of range for Intel Hex file)
Hence, I am using [Here](https://developer.arm.com/-/media/Files/downloads/gnu-rm/7-2017q4/gcc-arm-none-eabi-7-2017-q4-major-win32.exe?revision=732bae94-c929-403d-9520-0b2bccd81ad7?product=GNU%20Arm%20Embedded%20Toolchain,32-bit,,Windows,7-2017-q4-major).
I installed in `C:\gnu_arm`, and added `C:\gnu_arm\bin` to the system path.

* Open a new command prompt, and `pip install yotta`.

* There are currently (Jan 2019) some package incompatibilities, so we need to hack the following:-
```
python -m pip install pyopenssl==17.5.0
python -m pip install cryptography==2.1.4
```

* Close/open a new command prompt, and test that `yotta` 
now does something. The executable for Yotta seems to have been 
added in C:\Python27\scripts.

* Download SRecord from 
[Here](http://srecord.sourceforge.net/download.html) - 
and again, I just dropped the three executables 
in the install zip in `C:\Python27\Scripts`.

### Test the Compiler

You don't have to do this part to comppython ile our code; it's just
the process I went through first time to learn how to compile
anything. If you don't want to do this, then skip down to 
the epidemic section below.

* Change directory to somewhere you want to put micro:bit code. 
Clone the samples:
* `git clone https://github.com/lancaster-university/microbit-samples`
* `cd microbit-samples`
* `yt target bbc-microbit-classic-gcc`
* `yt build` - lots of deprecation warnings about C++ 11. Never mind.
* New files have appeared in 
`build\bbc-microbit-classic-gcc\source`
* Plug in a micro:bit. 
* Windows sometimes tries desperately hard to download drivers 
from Windows Update. I found the best thing to do was use a 
laptop and disconnect from WiFi.
* Assuming it connected as drive E:, do this:-
* `copy build\bbc-microbit-classic-gcc\source\microbit-samples-combined.hex E:`
* and watch the Microbit. Hopefully... 

### To compile the micro:epi projects

* In a command-prompt, move into [src/microbit-projects](../src/microbit-projects)
* Don't do a `yotta init` in case you've seen that somewhere - .yotta.json and module.json are already in the repo.
* `yotta install lancaster-university/microbit` gets all the code dependencies.
* Edit `microbit-projects\yotta_modules\microbit-dal\inc\core\MicroBitConfig.h` Search for `#define MICROBIT_BLE_ENABLED` and set to `0`
* The `build.bat` script copies files into the `source` folder ready for compiling, while only needing one copy of the dependencies...
* `build master` builds the master binary.
* `build minion` builds the minion binary.
* In either case, the resulting binary will be `build\bbc-microbit-glassic-gcc\source\projectname-combined.hex`
* To deploy that binary it to an attached micro:bit, for example: `deploy E:`

## Compiling Java code

* Assuming that you've installed a Java JDK from somewhere like [here](http://www.oracle.com/technetwork/java/javase/downloads/index.html), 
and you have `javac.exe` and `jar.exe` in your path, (perhaps by adding something like `C:\Program Files\Java\jdk1.8.0_181\bin` in Control Panel, System,
Advanced System Settings, Environment Variables, Path, and re-opening a command-window...)
* ... then each of the Java folders within [src/java-projects](../src/java-projects) has a `compile.bat` which you run to compile the code. 
* For the [make-stickers](../src/java-projects/make-stickers) and [multi-copy](../src/java-projects/multi-copy), the result is an executable JAR
file. For the [Slideshow](../src/java-projects/sliedshow), I haven't yet worked out how to make a working JAR file for a JAVAFx application
that wants to read arbitrary external files. So the result for that case is a bunch of class files.
