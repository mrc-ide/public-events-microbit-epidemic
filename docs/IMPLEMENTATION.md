# public-events-microbit-epdemic: Implementation

Here, we describe the "infrastructure" and methods used to
make the epidemic work. As you've seen, there are three components: 
a manager GUI written in python, which talks through
the serial port to a single micro:bit master, which talks by 
radio to up to 100 micro:bit minions.

## Introduction to the micro:bit capabilities:-

The architecture is determined mostly by the capabilities of micro:bits:-

* They can broadcast radio messages, which all micro:bits can read with a listen event.
* There is no built-in point-to-point message method, nor a built-in method of deriving the source of a message. 
Those things have to be programmed deliberately into the message. Whenever we describe below one minion talking to another, 
that is shorthand for broadcasts, with source and target ids enclosed in the message, and expanded by the listener, to see
if the message concerns them.
* Micro:bits can be members of a "group"; a pool which limits the sending/receiving of messages to within that group.
* Micro:bits are given a pseudo-random serial number at manufacture; it is not guaranteed to be unique, but according to 
[this](https://support.microbit.org/support/solutions/articles/19000070728-how-to-read-the-device-serial-number) you would
need 100k to 250k micro:bits in one place to expect a duplicate.
* They have a 5x5 LED screen that can scroll or print messages; you can set individual brightness per pixel to 255 levels.
* They have an accelerometer, a compass, and a light sensor, but we don't uses all these (yet). Extensions to our work here 
could include vaccination properties that vary with light, direction, or motion.
* They have a clock that counts the number of milliseconds since the minion is powered on, but no real-time clock. Hence, we 
have to manually synchronise.

## The Components

The components and communication are shown below.

<pre>  
                                                     -------------
                                              |------| minion 0  |-----|
                                              |      -------------     |
                                              |                        |
 ----------   serial     ----------   radio   |      -------------     |   radio
 | Laptop |--------------| master |-----------|------| minion 1  |-----|
 ----------              ----------           |      -------------     |
                                              |                        |
                                              |      -------------     |
                                              |------| minion 99 |-----|
                                                     -------------

</pre>

Additionally, if a minion is plugged into the laptop by USB/serial, it will
communicate to tell the laptop that it's a minion, and display its serial
number and software version. But this is for diagnostic/convenience only,
and serves no further purpose.

## Serial messages.

Messages over serial seemed to be interrupted at times by spurious new lines. The best solution
seemed to be to ignore new-lines, and introduce a standard end-of-message character: _#_.

### Identification

<pre>

 ----------              -----------          ----------   
 | Laptop |------------->| 1#      |--------->| master |
 |        |              -----------          |   or   |
 |        |                                   | minion |
 |        |     -----------------------|      |        |
 |        |     | [epi_version]:       |      |        |
 |        |<----| [serial_no]:         |<-----|        |
 |--------|     | [system_version]#    |      ----------
                ------------------------

</pre>

* `[epi_version]` is currently `VER:Epi Master 1.13` or `VER:Epi Minion 1.13`
* `[serial_no]` is the serial number of the micro:bit - `microbit_serial_number()`
* `[system_version]` is returned by `uBit.systemVersion()`

### Send Parameters

 ----------              -----------------------       ----------   
 | Laptop |              | 3[epid],[R0],       |       |        |
 |        |              | [rtype],[poimin],   |       |        |
 |        |------------->| [poimax],[rpower],  |------>| master |
 |        |              | [exposure],[btrans],|       |        |
 |        |              | [brec],[icons],#    |       |        |
 |--------|              -----------------------       ----------

* `[epid]` : unsigned short - epidemic id.
* `[R0]` : float - R0 parameter.
* `[rtype]` : unsigned char. 0 = Constant, 1 = Poisson
* `[poimin]` : unsigned char. Minimum permitted R.
* `[poimax]` : unsigned char. Maximum permitted R.
* `[rpower]` : unsigned char. Transmit power (0-7)
* `[exposure]` : unsigned short. Exposure threshold for infection (seconds)
* `[btrans]` : unsigned char. Button to enable transmit. (0,1,2,3) = (Auto, A, B, A+B)
* `[brec]` : unsigned char. Button to enable recovery. (0,1,2,3) = (Auto, A, B, A+B)
* `[icons]` : unsigned char Icon set. (0,1,2) = (SIR,I+R,-I-)

The serial port is assumed reliable, and the master doesn't acknowledge or reply to this
message. It changed mode into `MASTER_STAGE_RECRUITMENT`, and now listens to radio requests
for minions to join the game.


* In this game, the properties (parameters) of an epidemic are:-
  * Epidemic number. A simple numerical id, which distinguishes one epidemic from another. 
    This, coupled with the serial number of the master, allows multiple epidemics to be run close together
    without interference across epidemics run by different managers/masters, and ensures there is no straggling of
    minions from the previous epidemic, if you run more than once.
  * R0 - the number of new contacts made per victim. This can be set to a constant, or a poisson-distributed number; each 
    infected minion will make that many unique contacts. But a contact is not the same as a new infection; an infectious minion
    may have a contact who is recovered, so this counts as a contact, but the recovered, contacted minion won't make any further
    infections.
  * Exposure - the number of seconds of accumulate proximity required for a potential transmission to occur. These do not have to
    be contiguous.
  * Transmit power - effectively, this is how loudly the infected radio shouts to its neighbours that it is infected. This is a 
    number between 0 and 7 (-30dbm, to +4dbm), where 0dbm is rated at 20m range; -30dbm seems to be about a foot or so.

### The Manager

The manager is a GUI written in python, running on a laptop. It provides a nice front-end 
for setting the parameters and behaviour of the micro:bits and handling top-level communications 
so that the progress of the epidemic can be visualised. It performs these functions, in this order:-

* Locate Master micro:bit on the serial port, that is ready to start an epidemic.
* Configure the epidemic parameters and send to the master.
* Record which minions the master says are part of the epidemic.
* Choose the first victim (seed), and send to the master.
* Collect data on infections and recoveries from the master.
* Handle end of epidemic and restart, or power-off.

### The Master

The master micro:bit talks to the Manager via the serial port, and talks to the
minions over the simple radio. It performs these functions, in this order:-

* Identifies itself, on request, to the manager.
* Receives configuration parameters from the manager.
* Once it has parameters, it distributes those to minions who join
the game, and tells the manager who has joined. An epidemic is defined by the 
master's serial number, and a manager-set epidemic number.
* Receives seeding information for the epidemic via serial. It changes radio group (thus no more minions get added),
and broadcasts the seeding information over the radio.
* Receives radio information from minions who start making contacts, or recover, and forwards to manager.
* At any time, if it receives a reset/power-off message from the manager, it forwards to the minions, and enters a permanent sleep, until
it is physically rebooted.

### The Minions

The minions are the agents of the epidemic, and are attached to humans with a lanyard. They remember the parameters of the
epidemic, and also have an array of counters, one for each of the (potential) 100 minions in our epidemic, which count how many
infection messages they have received from each other minion. They perform these functions, in this order:-

* Periodically broadcast their presence (ie, their serial number) to the master, who either ignores or responds.
* Listen for a personal response from the master, giving them the parameters for the epidemic, and the ID. It also receives what the
  master micro:bit thinks the time is, so it can synchronise its clock.
* At this point, the minion changes radio group, and waits to be seeded or infected.
* A susceptible has a state, which is one of S,I or R - Susceptible (waiting for an infection), 
Infectious (making infections), or Recovered (it still receives infection requests, but doesn't make new ones of its own) 
* A __susceptible__ minion listens for a seeding message containing its ID, or a general infection message.
  * A seeding message is immediately responded to; the minion broadcasts an "I was infected" message (which the master hears),
    and enters the Infectious state.
  * An infection message increases the exposure counter for the source minion. If it reaches the parameterised threshold, then the
    minion broadcasts a message to the "infector", offering to be a contact. The infector may or may not confirm that...
  * If the susceptible hears an "infection confirmation" message targetted for itself, it changes state and becomes infectious, and
    broadcasts a message to the master telling it 
* An __infectious__ minion:
  * Broadcasts "I am infected", at the epidemic's radio strength, for anyone to hear.
  * It listens for replies from minions who have been sufficiently exposed, and reply with an offer to be a contact. When it
    receives them, it reduces the number of contacts left to make, and if that reaches zero, it ignores all future offers of contact,
    and enters the __recovered_ state, broadcasting a message to the master to say it has recovered.
  * It also continues to do the things a susceptible does - it listens for infection messages and offers to be a contact; the only
    difference is that it doesn't change state or behaviour if it receives the "infection confirmation" message.
* A __recovered__ minion:
  * Does the same things as a susceptible and infectious minion, regarding listening for infection messages and offering to be a contact,
    but it doesn't change state or behaviour if it receives the "infection confirmation" message.
* In __any__ stage:
  * If it receives a reset message, which resets the counters, and returns to the very start of the registry algorithm.
  * If it receives a power-off message, it enters a permanent sleep until it is physically rebooted.
