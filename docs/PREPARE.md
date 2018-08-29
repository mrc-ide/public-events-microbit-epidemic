# public-events-microbit-epdemic: Preparation

Assuming that you now have 100 or so micro:bits, assembled and glued,
this part of the guide deals with getting the software onto the 
micro:bits, collecting their serial numbers and labelling the 
labelled, and ready to wear

### Flashing the micro:bit Mbed firmware

* I'm not sure how essential this step is, but because I sometimes
noticed some instability in the USB and Serial connections, I
updated the firmware that manages the micro:bit's USB port. This bit is really tedious.

* Download the latest firmware [here](https://github.com/ARMmbed/DAPLink/releases) -
at time of writing, version 0247. Download the ZIP, and extract from it the
micro:bit firmware, which in this case is named _0247_kl26z_microbit_0x8000.hex_.

* For each micro:bit, I suggest insert USB cable _carefully_ into the
micro:bit, then hold the _RESET_ button (the one next to the USB port), at the same
time as you plug the USB cable into a desktop or laptop. 
* On Windows: The micro:bit attaches as a drive called _MAINTENANCE_. Copy the hex file on to that drive. 
The micro:bit will upgrade and reboot itself in non-maintenance mode.
* If you want to verify, examine the _DETAILS.TXT_ file on the micro:bit, and check
that _Interface Version_ matches what you upgraded.

### Installing the epidemic software

* The process of flashing code onto the micro:bit is similar, but can be made
much less tedious. The compiled binaries are in the [_bin/microbit-firmware/_](../bin/microbit-firmware/) folder of
this repo. Updating a single micro:bit is easy enough - attach the micro:bit by USB cable to
your computer, (don't hold the _RESET_ button, as you want it in normal flashing mode), and
copy the appropriate HEX file to the drive. You want one master, and many minions. 

* Alternatively, I wrote a tool to make this a bit easier...

### Using the Multi-Flasher

* You'll need Java installed to run this. I use the Java Development Kit, I suggest at
present a version 8 (currently I use Java SE 8u181, Windows 64-bit version). Download
from [here](http://www.oracle.com/technetwork/java/javase/downloads/index.html). 

* Run _MultiCopy.jar_ file in [bin/multi-copy/](../bin/multi-copy/) - in Windows you can double-click on it
if you've got a Java 8 installed. On other platforms, or from a Windows
command-prompt, something like _java -jar MultiCopy.jar_

* This basically detects a micro:bit plugged in, copies a specific firmware to it, waits
for the micro:bit to reboot, and then lets you unplug it. It works by detecting the drives
that are added, and is pretty simple. Incidentally, it doesn't work very well for flashing
the micro:bit's USB firmware, because the drive-letter assigned in maintenance mode was
often different from the one assigned in normal mode, which causes detection issues.

* Anyway, Browse for the firmware you want to flash (probably the minion if you want
to do it lots of times), tick the Enabled box, then start plugging in micro:bits, and
they'll be automatically flashed.

* Remmeber to disable/close the program before plugging in any more micro:bits.

### Serial numbers

* NExt, 

