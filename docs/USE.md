# public-events-microbit-epdemic: Usage - running an epidemic!

So far, you've bought, built and glued the hardware, installed Python, Java and R,
possibly upgraded the Mbed firmware in the micro:bits, flashed 100 or so with the 
minion firmware, and another one with the master firmware. You've got a serial port driver installed 
(if you're on windows), and you've got some/all your micro:bits identified with an id
between 0 and 99. It's been quite an emotional and character building experience, but 
you're now ready at last to run some epidemic games.

### The GUI tools

Connect to a projector or second screen, and run in Extended Desktop mode. We'll be running two graphical things at once:
(1) the micro:bit epidemic manager, which stays on the private screen, and (2) the Java slideshow,
which has a little control window on the private screen, and shows graphs full-screen on the
extended desktop. We'll describe them separately to preserve sanity, but in reality, you'll probably
start them both together, and most of the time let them run.

### The micro:bit manager

* Attach the master micro:bit to the host, and start the gui by running [bin/python-gui/run.bat](../bin/python-gui) 
(or `python epi_manager.py` from that folder).

* At this point (or earlier), ask the players to wear their micro:bit lanyards, fit the batteries as necessary, and
switch on. The micro:bits will display _U_, meaning they are unattached to an epidemic at the moment.

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

| Parameter       | Meaning                                                                                  |
| --------------- | ---------------------------------------------------------------------------------------- |
| R0              | The basic reproductive number                                                            |
| R Type          | If Constant, all infecteds make _R0_ contacts; if Poisson, then R0 is used as the mean   |
| Poi minimum     | If RType is Poisson, ensure number of contacts is >= Poi_min. Ignored if R Type Constant |
| Poi maximum     | If RType is Poisson, ensure number of contacts is <= Poi_max. Ignored if R Type Constant |
| Transmit range  | The radio power level (0-7) used for broadcasting infection status                       |
| Exposure (s)    | The number of seconds of (non-contiguous) contact that causes infection                  |
| Transmit button | Only broadcast infection when micro:bit buttons are pressed - or auto for continuous     |
| Receive button  | Only allow recovery when micro:bit buttons are presed - or auto for automatic reovery    |
| Icon Set        | What to display on micro:bits for the S-I-R states. See below...                         |

* You can save your parameter settings for recall later. Four sample games are included in the repo - see the
section below about those.

* Broadcast the parameters to the micro:bits by clicking the _Send Parameters_ button. At this point, id numbers
in the grid will start to turn green, as micro:bits join the game, and as they do so, their screen changed from _U_ 
to another letter that indicates their state - conventionally _S_ for susceptible, but see the parameters above for
other options. 

* One more thing: the _Epidemic ID_ is automatically incremented each time you start a new game, although you can
edit it if you want. Epidemic games are defined by this id, and the serial number of the master, hence you can
run multiple epidemics in the same space with different master micro:bits, and the games will be isolated from
each other. Similar, the incremental epidemic id makes sure that one game is isolated from the following game - 
this prevents issues where for example a player takes their minion out of radio range, and returns during the following game.

#### Screen 3: Seed the epidemic

* Seeding causes a susceptible player to suddenly become infected. Choose the victim by clicking on a green 
square in the grid, or click the _Random_ button to pick a random susceptible. 

* The number of victims that seed tries to infect can either be _forced_ to a number you define by ticking
the box, or if the _force_ box is left unticked, the number of victims the seed makes will be determined
from the parameters of the epidemic.

* Click the seed button to carry out the action, and the green square should turn red.

* You can re-seed the epidemic as often as you like.

* But note that as soon as the first seed is made, the players in the game are fixed, and no new micro:bits
can added themselves. They'll have to wait for the next game.

#### Ending the epidemic

* Click the _Reset_ button to end an epidemic game and start a new one. To confirm, you must type exactly _RESET_ or _POWEROFF_. 

* _RESET_ causes all micro:bits to return to the waiting room at the start. They will display the _U_ symbol
(unattached), and the microbit:manager returns to the first screen, where you choose which microbit:master to use.

* _POWEROFF_ will cause all micro:bits to blank their screen, turn off their radio, and basically go into a shut-down
state which is the most battery-efficient thing to do while still being switched on. To recover a minion from this mode,
it will need its own reset button pushed, or its power cycled, so do this at the end of the day. (Arguably, this might not
be the best way to end, since it's not easy to see whether micro:bits are on or off, when their screen is blank. Up to you!)

### The Java Slideshow

#### Getting started

* Launch by double-clicking on [bin/slideshow/run.bat](bin/slideshow) - or from a command-prompt or
terminal, `java MicroEpiSlideshow` in that folder. (Sorry it's a mess of class files; there seem to be issues
accessing external images when a JavaFX application is bundled into a JAR file, which are hard to get
around, since displaying arbitrary external images are an important part of what the slideshow does. 
I'll figure it out at some point and tidy this up)

* A small window appears, which lets you choose the location and size of the main display window, and
switch it on and off.

* Click _Detect Fullsreen_, to choose a monitor to display the fullscreen page onto.

* You can save this configuration for quick recall if you like.

* Click _Set Datafile_ to pick what file you want the slideshow app to monitor. These live in the [data/](data/) folder,
and are written by the micro:bit manager when epidemic games are running. The files will be named masterserial_epidemicid.csv,
and an XML file gives meta-data about the run.

* Click ON/OFF to turn the display window on and off!

* While the slideshow is running, and if either the player window, or the Slideshow interface are in focus, you can press _Q_ 
to skip through the slideshow pages, ignoring any delays, or any other key will pause or unpause the slideshow, displaying a 
paused icon in the top-right.

#### The Script File

If you want to make your own epidemic games, or your own slideshow sequences, this section is how...

* The name of a parameter set is important; the micro:bit manager writes it to the XML meta-data for a particular
epidemic, and the Java Slideshow loads that meta-data, and looks for a script file called script_paramset.txt - where _paramset_
is the name of the parameter set.

* So for each parameter set (ie, each game type), you should write a script file in a text editor. Scripts are simple text, read
line by line sequentially, and will end with a _LOOP_ command, which restarts the sequence. Here are the commands that are 
understood by the Slideshow interpreter. Commands are case-insensitive, although file names and paths on Mac/Linux platforms will be
case sensitive.

| Setup/misc commands |
| ------------------- |
| *# Comment*         |
| This is a comment - ignored.
| *DataPath:../../data* |
| Set the default place to find the CSV and XML files. |
| *Language:EN *|
| The slideshow allows multi-language support... |
| *Loop* |
| Start the script again from the top |
| *RnetGraph:Outbreak7.R* |
| Sets the R script to call, to create the network image, (called staticnetworkgraph.png). |
| *RScript:C:/Program Files/R/R-3.5.1/bin/RScript.exe* |
| Where is RScript.exe? The above is typical (and necessary) for Windows; on Mac/Linux, Rscript:Rscript is fine.|
| *Timezone:GMT+1* |
| What timezone to use for interpreting universal timestamps  |
| *Wait 10* |
| Pause for a number of seconds |

| Display commands |
| ---------------- |
| *CasesGraph* (optionally add: *Cumulative* and/or *Unconfirmed* |
| Plot incidence, optionally cumulative. See below regarding _Unconfirmed_ |
| *GenTimeGraph* |
| Plot a graph showing the distribution of times from being infected to making an infection |
| *Image["image1.png","image2,png"]* - as many images as you like |
| Display one image. Next iteration, play the subsequent image in the list |
| *Movie["movie1.mp4","movie2.mp4"]* - as many movies as you like |
| Play one movie. Next iteration, play the subsequent movie in the list |
| *NetworkGraph* |
| Refresh/display the network graph of the epidemic. |
| *R0Graph* (optionally add: *Unconfirmed*)
| Plot a graph of how many people made _x_ contacts. See below regarding _Unconfirmed_ |
| *Spreaders* |
| Display a leaderboard of who has made the most victims |
| *Status Susctible,Infected,Recovered* |
| Display stats on how many players are in the three states - and label them as above. |
| *Survivors* |
| Display a dramatic page about who the last survivor is, or how many are left. |
| *TreatmentsGraph* (optionally add: *Cumulative* and/or *Unconfirmed* |
| As _CasesGraph_ but with labels that work with the _Saviour_ game - see below |

### Sample Epidemic games

#### Basic epidemic

#### Saviour

#### Survivor

#### Super-Spreader

