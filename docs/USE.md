# public-events-microbit-epdemic: Usage - running an epidemic!

So far, you've bought, built and glued the hardware, installed Python, Java and R,
possibly upgraded the mBed firmware in the micro:bits, flashed 100 or so with the 
minion firmware, and another one with the master firmware. It's been quite an 
emotional and character building experience. You've got a serial port driver installed 
(if you're on windows), and you've got some/all your micro:bits identified with an id
between 0 and 99. You're now ready at last to run some epidemic games.

### The GUI tools

Connect to a projector and run in Extended Desktop mode. We'll be running two graphical things at once:
(1) the micro:bit epidemic manager, which stays on the private screen, and (2) the Java slideshow,
which has a little control window on the private screen, and shows graphs full-screen on the
extended desktop. We'll describe them separately to preserve sanity, but in reality, you'll probably
start them both together, and most of the time let them run.

### The micro:bit manager

* Attach the master micro:bit to the host, and start the gui by running [bin/python-gui/run.bat](../bin/python-gui) 
(or `python epi_manager.py` from that folder).

#### Screen 1: Select master
* On the first screen, you select which COM port the master is connected to; the manager detects all available 
options, which must be a master micro:bit of a software version it likes.
* You can, incidentally, run multiple epidemic games at the same time, even from the same laptop, but you need one
master per epidemic, and one micro:bit manager instance per epidemic. And a good ability to multitask. 
An epidemic is defined by the serial number of the master micro:bit, and a user-set epidemic number.

#### Screen 2: Define parameters

* On the next page, the parameters are set for a specific epidemic. The set of parameters can be saved and
recalled later. Some of them may become more clear when we talk later about the epidemic games we've made so far.
But briefly:-

#### Screen 3: Seed the epidemic

#### Ending the epidemic

### The Java Slideshow

#### Getting started

#### The Script File

### Sample Epidemic games

#### Basic epidemic

#### Saviour

#### Survivor

#### Super-Spreader

