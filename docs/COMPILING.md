# Microbit-Epidemic: Compiling Guide

Here, we describe how to create an environment on windows, 
for compiling our code into binaries to copy to the micro:bit.
In due course, we'll try this on other platforms, which will
probably be quite similar on the whole.

## Compiler Tools on Windows

* Following the instructions for yotta...

* The "simple" installer didn't work for me - `yt` and `yotta` 
didn't seem to exist, even though the `Run Yotta` shortcut 
seemed to be present. So, I followed the manual instructions 
to get it working, namely...
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

## Test the Compiler

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

## To make some new code

Again, this is for the learning experience, and not necessary
if you want to cut to the chase and compile our code.

* Create and move into the base folder for the project.
* `yotta init`
* Answer a few questions about the module name, start version, 
whether it is an executable or a library, description, author 
and license.
* Some init files are created, along with a `source` and 
`test` folder.
* `yotta target bbc-microbit-classic-gcc` to set the 
compilation target.
* `yotta install lancaster-university/microbit` will install 
the entire dependency library for the project.
* Edit `microbit-projects\yotta_modules\microbit-dal\inc\core\MicroBitConfig.h` 
Search for `#define MICROBIT_BLE_ENABLED` and set to `0`
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
* Typically, Windows takes a while to install drivers from Windows Update, for each USB port you plug into, so do this with
some time and internet connection in hand.
* Then I wrote some R code to read the serial port:-

```R
library(serial)

con <- serialConnection(
  name = "COM8",
  port = "COM8",
  mode = "115200,n,8,1",
  buffering = "none")

open(con)
while (TRUE) {
  incoming <- read.serialConnection(con)
  if (nchar(incoming)>0) {
    message(incoming)
  }
  Sys.sleep(0.1)
}
close(con)
```

* Note that on Windows, COM ports are virtual, and a new one gets created very freely - eg, when an unrecognised device gets added. Unrecognised
could mean a new micro:bit not previously plugged in, or a familiar micro:bit but with upgraded firmware.

## Upgrading micro:bit firmware

* I don't know how important this is, but as part of trying to solve Windows searching for drivers for several minutes, I tried upgrading the
firmware.
* Download latest firmware [here](https://support.microbit.org/support/solutions/articles/19000019131-how-to-upgrade-the-firmware-on-the-micro-bit)
* As in the instructions, plug in micro:bit by USB while holding the reset button.
* Copy the firmware to the `Maintenance` drive letter that pops up.
* On one occasion, a micro:bit refused to connect in maintenance mode, with an "Unrecognised USB Device" sort of error. This cleared 
up after the first normal code flashing I did.
