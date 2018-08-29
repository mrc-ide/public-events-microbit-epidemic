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

### Stickers

* We chose to print a sticker for each micro:bit, which would give its id number, and
a little message of where to send it, should it get lost and found. A little utility is
included to make some images you can print, which prepares some pages for 2.5" x 1.5" sheets
of labels - 21 per page.

* Having installed java, for the default behaviour, run [bin/make-stickers/MakeStickers.jar](../bin/make-stickers/),
either by double-clicking it in Windows, or `java -jar MakeStickers.jar` from a command-prompt
or terminal. By default, it will create 5 PNG files, which when printed as full A4-size pages
will land nicely on the stickers.

* To customise the image, make an edit of [bin/make-stickers/sticker.png](../bin/make-stickers/sticker.png).

* The default behaviour is the same as running `java -jar MakeStickers.jar 0 99 sticker.png page` - the arguments are
the first and last number to print, the background image, and the stub for the output filenames.

### Serial numbers

* Next, attach the master micro:bit to the serial port, and run the epidemic management 
utility from [bin/python-gui/run.bat](../bin/python-gui) - the batch file simply runs
`python epi_manager.py`. The code was developed with Python 2.7 in mind, although it
may well run on more recent pythons. Get your ~~albatross~~ Python 2.7 installer [here](https://www.python.org/downloads/release/python-2715/) - 
I used the Windows 64-bit installer.

* More detail on this utility in the [usage](use.MD) document. For now, the utility should
detect the micro:bit master which you plugged in - or click rescan to have another look on
the USB ports. And you can then click the _Set Master micro:bit_ button in the bottom left.

* The next page shows all the parameters for the epidemic. Ignore them and click _Send Parameters_

* Switch on your minion micro:bits one at a time. The serial number of the micro:bit is recorded in [bin/python-gui/serials.csv](../bin/python-gui/serials.csv), and
links a long micro:bit serial number with an id (0-99). This relationship persists until you delete the serials.csv file. When you power up a micro:bit, a square
on the grid will turn green, revealing the id number (either newly assigned, or remembered) forthat micro:bit.

* Note that micro:bit serial numbers are not *perfectly* unique, technically speaking, but should be amply close enough unless you're
extraordinarily unlucky. See [here](https://support.microbit.org/support/solutions/articles/19000070728-how-to-read-the-device-serial-number). 
I haven't coded any specific provision for duplicate serial numbers.
