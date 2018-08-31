# public-events-microbit-epdemic: Compiling Guide

Here, we describe how to create an environment on windows, 
for compiling our code into binaries to copy to the micro:bit.
In due course, we'll try this on other platforms, which will
probably be quite similar on the whole.

## Compiling for micro:bit

### Compiler Tools on Windows

I wanted to do offline compilation of C code to Microbit, on Windows. So,
I used these [Instructions](https://lancaster-university.github.io/microbit-docs/offline-toolchains/) which
pointed me to [here](http://docs.yottabuild.org/#installing-on-windows).

* The "simple" installer didn't work for me - `yt` and `yotta` 
didn't seem to exist, even though the `Run Yotta` shortcut 
seemed to be present. So, I followed the [manual](http://docs.yottabuild.org/#manual-windows-installation)
instructions and got it working:-

* Install latest Python 2.7, 64-bit version. 
[Here](https://www.python.org/ftp/python/2.7.14/python-2.7.14.amd64.msi). I installed into 
C:\Python27. I didn't get the option in the installer to add 
this to the path, so... Control Panel, System, Advanced System 
Settings, Environment Variables, find Path in the System 
variables, prefix with `C:\Python27;C:\Python27\Scripts;`

* Upgrade pip, because... why not? New command window, 
`python -m pip install --upgrade pip`

* Install CMake. I went for 
[this](https://cmake.org/files/v3.11/cmake-3.11.1-win64-x64.msi). It let me add cmake to the path for
all users. I put it in `C:\CMake`

* Install [Ninja](https://github.com/ninja-build/ninja/releases)
- I took the latest version, 1.8.2. It's a single executable 
file, so I cheated a bit, and put it in my Python scripts 
folder, which is already in the path. Yes, I know it's not a 
python script.

* Install the gcc cross-compiler (gcc-arm-none-eabi). The 
Yotta notes talk about an old version 4.9; I'm trying out the 
latest from [Here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads),
which seems to be 7-2017-q4-major. I've installed in 
`C:\gnu_arm` - keeping directory names simple is probably wisdom.
And this one also has an option to automatically add 
`C:\gnu_arm\bin` to my path, so I let it.

* Open a new command prompt, and `pip install -U yotta`.

* Close/open a new command prompt, and test that `yotta` 
now does something. The executable for Yotta seems to have been 
added in C:\Python27\scripts.

* Download SRecord from 
[Here](http://srecord.sourceforge.net/download.html) - 
and I cheated again, and just dropped the three executables 
in the install zip in `C:\Python27\Scripts`.

### Test the Compiler

You don't have to do this part to compile our code; it's just
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
and you have `javac.exe` and `jar.exe` in your path, (perhaps by adding `C:\Program Files\Java\jdk1.8.0_182\bin` in Control Panel, System,
Advanced System Settings, Environment Variables, Path, and re-opening a command-window...)
* ... then each of the Java folders within [src/java-projects](../src/java-projects) has a `compile.bat` which you run to compile the code. 
* For the [make-stickers](../src/java-projects/make-stickers) and [multi-copy](../src/java-projects/multi-copy), the result is an executable JAR
file. For the [Slideshow](../src/java-projects/sliedshow), I haven't yet worked out how to make a working JAR file for a JAVAFx application
that wants to read arbitrary external files. So the result for that case is a bunch of class files.
