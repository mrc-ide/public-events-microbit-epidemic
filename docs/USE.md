# public-events-microbit-epdemic: Common Usage

So far, you've bought, built and glued the hardware, installed Python, Java and R,
possibly upgraded the Mbed firmware in the micro:bits, flashed 100 or so with the 
minion firmware, and another one with the master firmware. You've got a serial port driver installed 
(if you're on windows), and you've got some/all your micro:bits identified with an id
between 0 and 99. It's been quite an emotional and character building experience, but 
you're now ready at last to run some epidemic games.

## The GUI tools

Connect to a projector or second screen, and run in Extended Desktop mode. We'll be running two graphical things at once:
(1) the micro:bit epidemic manager, which stays on the private screen, and (2) the Java slideshow,
which has a little control window on the private screen, and shows graphs full-screen on the
extended desktop. We'll describe them separately to preserve sanity, but in reality, you'll probably
start them both together, and most of the time let them run.

## The micro:bit manager

* Attach the master micro:bit to the host, and start the gui by running [bin/python-gui/run.bat](../bin/python-gui) 
(or `python epi_manager.py` from that folder).

* At this point (or earlier), ask the players to wear their micro:bit lanyards, fit the batteries as necessary, and
switch on. The micro:bits will display _U_, meaning they are unattached to an epidemic at the moment.

### Screen 1: Select master

* On the first screen, you select which COM port the master is connected to; the manager detects all available 
options, which must be a master micro:bit of a software version it likes.

* You can, incidentally, run multiple epidemic games at the same time, even from the same laptop, but you need one
master per epidemic, and one micro:bit manager instance per epidemic. And a good ability to multitask. 
An epidemic is defined by the serial number of the master micro:bit, and a user-set epidemic number.

### Screen 2: Define parameters

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

### Screen 3: Seed the epidemic

* Seeding causes a susceptible player to suddenly become infected. Choose the victim by clicking on a green 
square in the grid, or click the _Random_ button to pick a random susceptible. 

* The number of victims that seed tries to infect can either be _forced_ to a number you define by ticking
the box, or if the _force_ box is left unticked, the number of victims the seed makes will be determined
from the parameters of the epidemic.

* Click the seed button to carry out the action, and the green square should turn red.

* You can re-seed the epidemic as often as you like.

* But note that as soon as the first seed is made, the players in the game are fixed, and no new micro:bits
can added themselves. They'll have to wait for the next game.

### Ending the epidemic

* Click the _Reset_ button to end an epidemic game and start a new one. To confirm, you must type exactly _RESET_ or _POWEROFF_. 

* _RESET_ causes all micro:bits to return to the waiting room at the start. They will display the _U_ symbol
(unattached), and the microbit:manager returns to the first screen, where you choose which microbit:master to use.

* _POWEROFF_ will cause all micro:bits to blank their screen, turn off their radio, and basically go into a shut-down
state which is the most battery-efficient thing to do while still being switched on. To recover a minion from this mode,
it will need its own reset button pushed, or its power cycled, so do this at the end of the day. (Arguably, this might not
be the best way to end, since it's not easy to see whether micro:bits are on or off, when their screen is blank. Up to you!)

## The Java Slideshow

This is a Java and JavaFX based slideshow, that shows potentially interesting graphs and displays while the epidemic
is running. It was originally written for a slightly different epidemic game with different rules, hence a few artefacts
survive here that are not specifically useful for the micro:bit epidemic, but are described here anyway.

### Getting started

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

### The Script File

If you want to make your own epidemic games, or your own slideshow sequences, this section is how...

* The name of a parameter set is important; the micro:bit manager writes it to the XML meta-data for a particular
epidemic, and the Java Slideshow loads that meta-data, and looks for a script file called script_paramset.txt - where _paramset_
is the name of the parameter set.

* So for each parameter set (ie, each game type), you should write a script file in a text editor. Scripts are simple text, read
line by line sequentially, and will end with a _LOOP_ command, which restarts the sequence. Here are the commands that are 
understood by the Slideshow interpreter. Commands are case-insensitive, although file names and paths on Mac/Linux platforms will be
case sensitive.

| Script structure |
| ---------------- |
| Setup commands|
| **SCRIPT:**|
| Script commands|
| **LOOP:**|

| Setup commands |
| ------------------- |
| **# Comment**         |
| This is a comment - ignored at any time.
| **DataPath:../../data** |
| Set the default place to find the CSV and XML files. |
| **Language:EN**|
| The slideshow allows multi-language support... |
| **Loop** |
| Start the script again from the **Script:** command |
| **RnetGraph:Outbreak7.R** |
| Sets the R script to call, to create the network image, (called staticnetworkgraph.png). |
| **RScript:C:/Program Files/R/R-3.5.1/bin/RScript.exe** |
| Where is RScript.exe? The above is typical (and necessary) for Windows; on Mac/Linux, Rscript:Rscript is fine.|
| **Script:** |
| Marks the end of the setup, and the start of the display script.
| **Timezone:GMT+1** |
| What timezone to use for interpreting universal timestamps |

| Display commands |
| ---------------- |
| **CasesGraph** (optionally add: **Cumulative** and/or **Unconfirmed** |
| Plot incidence, optionally cumulative. See below regarding _Unconfirmed_ |
| **GenTimeGraph** |
| Plot a graph showing the distribution of times from being infected to making an infection |
| **Image["image1.png","image2,png"]** - as many images as you like |
| Display one image. Next iteration, play the subsequent image in the list |
| **Movie["movie1.mp4","movie2.mp4"]** - as many movies as you like |
| Play one movie. Next iteration, play the subsequent movie in the list |
| **NetworkGraph** |
| Refresh/display the network graph of the epidemic. |
| **R0Graph** (optionally add: **Unconfirmed**)
| Plot a graph of how many people made _x_ contacts. See below regarding _Unconfirmed_ |
| **Spreaders** |
| Display a leaderboard of who has made the most victims |
| **Status Susctible,Infected,Recovered** |
| Display stats on how many players are in the three states - and label them as above. |
| **Survivors** |
| Display a dramatic page about who the last survivor is, or how many are left. |
| **TreatmentsGraph** (optionally add: **Cumulative** and/or **Unconfirmed**) |
| As _CasesGraph_ but with labels that work with the _Saviour_ game - see below |
| **Wait 10** |
| Pause for a number of seconds |


* The Spreaders, Status and Survivors pages were specifically hacked for the micro:bit epidemic,
and rely on [bin/slideshow/media/template.png](bin/slideshow/media/template.png) as a backdrop. 

* The _Unconfirmed_ flag. This includes in the figures contacts that are not necessarily infections,
thus providing a sort of _worst case_. The epidemic works by deciding how many unique contacts an infected
player will make at the beginning of their 'infectious period'. The contacts are chosen in the micro:bit
epidemic, based on their exposure reaching a certain threshold. At that time, the contact could be in any
of the three states; susceptible, infected, or recovered, since they may have been infected at any previous 
moment by some other infector. In that case, they still count as a _contact_, but they don't get _infected_
again. (Essentially, this is the herd immunity principle in action).

* So.. **CasesGraph Unconfirmed** will plot incidence, but will also include the _worst-case_ scenario; we 
know how many contacts will be made by the currently infected people, so the _worst-case_ is what happens if
every one of those contacts is a susceptible who thus becomes infected.

* Similarly: **R0Graph Unconfirmed** will include on the graph the _worst-case_ - how many people will
make 'n' infections, assuming that every contact made results in an infection.

* In practise... the difference between the _worst-case_ and the actual outcomes of the epidemic are 
interesting academically, but they can make the graphs complicated, and take some time to explain what
they mean. So only use it if those are the sort of discussions you want to have.

### The R Network Script

* See [src/r-projects/NetworkGraph](src/r-projects/NetworkGraph] - the script is then copied into 
[bin/slideshow](bin/slideshow) for convenience in writing the scripts.

* If you want to rewrite or improve the script, it takes seven arguments:-

| No | Description | Example |
| -- | ----------- | ------- |
| 1 | The data file (csv) to read. | ../data/498461974_1.csv |
| 2 | The width of png file to create. | 1024 |
| 3 | The height of png file to create | 768 |
| 4 | The colour of category 1* | #000000 |
| 5 | The colour of category 2* | #000000 |
| 6 | The colour of category 3* | #000000 |
| 7 | The filename to produce | staticnetworkplot.png |

* The categories are legacy and aren't used in the micro:bit epidemic at present.

* The Slideshow software then pastes a key onto this graph, since this was fiddly to do in R in a screen-ready way.

### The CSV File Format

* The CSV file has one row of headers, and a number of lines afterwards. It records a list of infections and recoveries.

| Column | Meaning |
| ------ | ------- |
| Event | **I** or **R**, for infection or recovery |
| TimeH | Time of day (hours) of event | 
| Mins | Minutes of day (floating point) of event |
| InfectedBy | Id who infected me (for Infection event. **NA** for recoveries) |
| Seeding | **S** for a seeded infection, N for normal infection, **NA** for recoveries |
| Recency | **Recent** means an infection was recent. Anything else old. **NA** for recoveries.  |
| Category | **1**, **2** or **3** - categories of people. But *ignored* for micro:bit epidemic.| 
| ID | ID of the infected/recovered player |
| NoContacts | Number of contacts to be made for infection; NA for recovery |

## Sample Epidemic games

* This repo 'ships' with four epidemic games which we've used. There are four accompanying
scripts for the Slideshow to use, and various images in [../bin/slideshow/media](../bin/slideshow/media) 
support the four games.

### Basic epidemic

| Parameter       | Value   |
| --------------- | ------- |
| R0              | 2.4     |
| R Type          | Poisson |
| Poi minimum     | 1       |
| Poi maximum     | 5       |
| Transmit range  | 4 or 5  |
| Exposure (s)    | 120     |
| Transmit button | Auto    |
| Receive button  | Auto    |
| Icon Set        | SIR     |

* This is the basic epidemic, with a medium range infection, taking a couple of minutes of accumulated contact to 
cause an infection. In a social gathering, it will probably take around 15-20 minutes.

* Seeding: Single seed, and allow to pick number of contacts from poisson. Reseed as necessary to keep things moving.

### Survivor

* This is the same as the basic epidemic, but the Slideshow will show an extra page showing how many
survivors are left, and if there is one (or less), then who was the last player to be susceptible.

* Seeding: similar to the basic epidemic, but if it seems to have died out, reseed again.

* Note that this is entirely unfair, as games go, and in the latter stages of seeding is probably dictated by chance...

### Saviour

| Parameter       | Value   |
| --------------- | ------- |
| R0              | 0       |
| R Type          | Constant|
| Poi minimum     | 0       |
| Poi maximum     | 1       |
| Transmit range  | 0       |
| Exposure (s)    | 1       |
| Transmit button | A       |
| Receive button  | B       |
| Icon Set        | I+R     |

* This game is a bit of a variation, using all the same internal mechanics, but with some creative labelling, we can 
make a different sort of game. The _Icon Set_ is particularly interesting (and confusing!) here; it allows us to label
susceptibles, infected, and recovered people with whatever letter we like, even though internally, the same S-I-R sort
of rules are applying.

* Everyone in this game begins with an infection, and their micro:bits say _I_. One person discovers a cure, and becomes
a doctor - a _+_ symbol appears on their micro:bit. A doctor (_+_) can treat an "infected" (_I_) if the two stand close
together, the doctor presses the 'A' button, and the patient presses the 'B' button, for a second, simultaneously. The
_I_ then becomes an _R_ signalling instant recovery.

* So the game works with somer bizarre mapping of what is displayed, to what is understood internally by the software.
What the micro:bits call _I_ is represented internally by susceptibles; the doctor is actually the one-and-only-infected
host, and recoveries are indeed recovered. As R0 is zero, the only way to become a doctor, is by seeding. And when the
doctor passes on the "infection", the "victim" immediately recovers, for the same reason.

* So to run the game, you just need to **Seed** the doctor - pick a green micro:bit, and seed them, forcing them to make 
99 contacts, which effectively means the doctor never runs out. 

* (For varieties on this game, you could limit the number of "vaccines" a doctor has, by changing how many contacts you force
them to make to a smaller number. Then you could seed more doctors later on in the epidemic).

### Super-Spreader

| Parameter       | Value   |
| --------------- | ------- |
| R0              | 99      |
| R Type          | Constant|
| Poi minimum     | 98      |
| Poi maximum     | 99      |
| Transmit range  | 2       |
| Exposure (s)    | 30      |
| Transmit button | Auto    |
| Receive button  | Auto    |
| Icon Set        | SIR     |

* This one is like the basic epidemic, except no-one ever recovers, and there is no limit on how many infections you can make.

* This is another quite unfair game! Those seeded or infected earlier have an extreme advantage that probably cannot be
overcome even by the most social people in the room. But fairness is probably not the main objective of these games.
