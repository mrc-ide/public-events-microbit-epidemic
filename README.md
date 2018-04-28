# public-events-microbit-epidemic
Proximity-driven epidemic over bluetooth

## Compiler Tools on Windows

* The "simple" installer didn't work for me - `yt` and `yotta` didn't seem to exist, even though the `Run Yotta` shortcut seemed to be
present. So, I followed the manual instructions to get it working, namely...
* Install latest Python 2.7, 64-bit version. [Here](https://www.python.org/ftp/python/2.7.14/python-2.7.14.amd64.msi). I installed into 
C:\Pyton27. I didn't get the option in the installer to add this to the path, so... Control Panel, System, Advanced System Settings, 
Environment Variables, find Path in the System variables, prefix with `C:\Python27;C:\Python27\Scripts;`
* Upgrade pip, because... why not? New command window, `python -m pip install --upgrade pip`
* Install CMake. I want for [this](https://cmake.org/files/v3.11/cmake-3.11.1-win64-x64.msi). This one let me add cmake to the path for
all users. I put it in `C:\CMake`
* Install [Ninja](https://github.com/ninja-build/ninja/releases) - I took the latest version, 1.8.2. It's only one executable file, so I
cheated a bit, and put it in my Python scripts folder, which is already in the path. Yes, I know it's not a python script.
* Install the gcc cross-compiler (gcc-arm-none-eabi). The Yotta notes talk about an old version 4.9; I'm trying out the latest from [Here](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads),
which seems to be 7-2017-q4-major. I've installed in `C:\gnu_arm` - I have the feeling keeping directory names simple might be good.
And this one also has an option to automatically add `C:\gnu_arm\bin` to my path.
* Open a new command prompt, and `pip install -U yotta`.
* Close/open a new command prompt, and `yotta` does something. Yotta has been added in C:\Python27\scripts.
* Download SRecord from [Here](http://srecord.sourceforge.net/download.html) - and I cheated again, and just dropped the three executables 
in the install zip in `C:\Python27\Scripts`.

## Test the Compiler

* Change directory to somewhere you want to keep microbit code. And clone the samples:
* `git clone https://github.com/lancaster-university/microbit-samples`
* `cd microbit-samples`
* `yt target bbc-microbit-classic-gcc`
* `yt build` - lots of deprecation warnings about C++ 11. Never mind.
* New files have appeared in `build\bbc-microbit-classic-gcc\source`
* Plug in a Microbit. Assuming it connected as drive E:, do this:-
* copy build\bbc-microbit-classic-gcc\source\microbit-samples-combined.hex E:
* and watch the Microbit. Hopefully... 

## To start new code

* Create and move into the base folder for the project.
* Answer a few questions about the module name, start version, whether it is an executable or a library, description, author and license.
* Some init files are created, along with a `source` and `test` folder.
* `yotta target bbc-microbit-classic-gcc` to set the compilation target.
* `yotta install lancaster-university/microbit` will intsall the entire dependency library for the project. (Overkill?)
* Create some code - eg `source/main.cpp` - and example code:
```C
#include "MicroBit.h"

MicroBit uBit;

int main() {
  uBit.init();
  uBit.display.scroll("Asparagus!");
  release_fiber();
}
```

* Build it with `yotta build` in the base folder of the project. 
* Plug in Microbit
* Send to the Micro:Bit with copy build\bbc-microbit-glassic-gcc\source\projectname-combined.hex E:

## Enabling data collection by Serial.

* Download Serial port driver on the receiving computer, from [Here](https://os.mbed.com/handbook/Windows-serial-configuration)
* Install instructions are basically: plug in Microbit, close any explorer windows, then run the installer.
* Look in Control Panel, Device Manager, Ports (COM & LPT), and note the COM number for "mBed Serial Port"