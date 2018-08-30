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

## Messages

* Messages over serial are supposed to be sent from buffers when a new-line is sent. However, spurious 
new-lines seemed to be commonly inserted. The best solution
seemed to be to send serial messages ending with a new-line, but also to include our own end-of-message character: _#_. New lines
are then used purely to trigger a buffer flush and send; they are stripped from incoming messages, and buffered until a _#_ is read,
which indicates the message has been received completely. Any non-newline left-over after the _#_ is buffered as the start of the
next message.

* Messages over radio are broadcast at a certain power, and on a certain group. Groups are designed to separate messages so that
they are only received by those interested in them. We use two groups, one for _REGISTERED_ micro:bits, and another for _UNREGISTERED_ - which
group messages are being sent in will be shown on each diagram. Radio power is assumed to be maximum, unless otherwise specified; the 
infection broadcast is the message where the power level is a parameter of the epidemic.

* Messages over radio appear to be limited to 28 bytes - not the 32 that the micro:bit documentation reports. 

* Chars are a byte long, shorts are 2 bytes long, and int and floats are 4 bytes long.

* See [src/microbit-projects/include/microepi.h](src/microbit-projects/include) for code that represents all of these messages.

### Identification of micro:bit

Here, the laptop sends a requst to a connected micro:bit, asking it to identify itself in various ways.

<pre>

 ----------              |---------|          ----------   
 | Laptop |------------->| 1#      |--------->| master |
 |        |              |---------|          |   or   |
 |        |                                   | minion |
 |        |     |----------------------|      |        |
 |        |     | [epi_version]:       |      |        |
 |        |<----| [serial_no]:         |<-----|        |
 |--------|     | [system_version]#    |      ----------
                |----------------------|

</pre>

* `[epi_version]` is currently `VER:Epi Master 1.13` or `VER:Epi Minion 1.13`
* `[serial_no]` is the serial number of the micro:bit - `microbit_serial_number()`
* `[system_version]` is returned by `uBit.systemVersion()`

### Send Parameters to Master

Here, the laptop sends the set of parameters for the epidemic, to the master. After our
new-line hack, the serial is assumed reliable (and no further issues have been observered).
The master therefore doesn't acknowledge or reply to this message explicitly. It now listens 
(on the UNREGISTERED group) to radio requests for minions who want to join the game.

<pre>
 ----------              |---------------------|       ----------   
 | Laptop |              | 3[epid],[R0],       |       |        |
 |        |   serial     | [rtype],[poimin],   |       |        |
 |        |------------->| [poimax],[rpower],  |------>| master |
 |        |              | [exposure],[btrans],|       |        |
 |        |              | [brec],[icons],#    |       |        |
 ----------              |---------------------|       ----------
</pre>

* `[epid]` : unsigned short - epidemic id.
* `[R0]` : float - R0 parameter.
* `[rtype]` : unsigned char. 0 = Constant, 1 = Poisson
* `[poimin]` : unsigned char. Minimum permitted R.
* `[poimax]` : unsigned char. Maximum permitted R.
* `[rpower]` : unsigned char. Transmit power (0-7)
* `[exposure]` : unsigned short. Exposure threshold for infection (seconds)
* `[btrans]` : unsigned char. Button to enable transmit. (0, 1, 2, 3) = (Auto, A, B, A+B)
* `[brec]` : unsigned char. Button to enable recovery. (0, 1, 2, 3) = (Auto, A, B, A+B)
* `[icons]` : unsigned char. Icon set. (0, 1, 2) = (SIR, I+R, -I-)

### Registration of a minion

Minions, on startup, are in the UNREGISTERED radio group, and they broadcast their 
willingness to play the epidemic game, sending their serial number and software version
to all who listen. But no-one listenes, until the master is ready. It then listens on the
same gruop, and on receiving a message, asks the laptop
for the friendly-id for this minion's (longer) serial number. The laptop replies, repeating the
minion's serial number, and returning the friendly id to the master. The
master broadcasts the minion's serial number (so the minion knows the
message is intended for it), the friendly id, the master's serial number, the
current time on the master, and the parameters of the epidemic.

The minion is now registered, becomes susceptible, changes message channel to
help ignore other minion's registration radio noise, and waits to be infected
or seeded.

<pre>
 ----------  radio   |---------------------|  radio   ----------       |------------------|        ----------
 | Minion |  bcast   | [REG_MSG]   : char  |  bcast   |        |       | REG:[serial_no]: | serial | Laptop |
 |        |   on     | [serial_no] : int   |   on     |        |       | [build_no]#      |        |        |
 |        |- - - - ->| [build_no]  : char  |- - - - ->| master |------>|                  |------->|        |
 |        |  UNREG   |                     |  UNREG   |        |       |------------------|        |        |
 |        |  group   |---------------------|  group   |        |                                   |        |
 |        |                                           |        |       |------------------|        |        |
 |        |<- |    |-----------------------|          |        |<------| 2[serial_no],    |<-------|        |
 ----------        | [REG_ACK_MSG] : char  |<- - - - -|        |       | [friendly_id],#  |        ----------
              |    | [serial_no]   : int   |          ----------       |------------------|
                   | [friendly_id] : short |
              |    | [master_ser]  : int   |
                   | [master_time] : int   |
              |    | [epid]        : short |
              - - -| [R0]          : float |
                   | [rtype]       : char  |
                   | [poimin]      : char  |
                   | [poimax]      : char  |
                   | [rpower]      : char  |
                   | [exposure]    : short |
                   | [bcombine]    : char  |
                   |-----------------------|

</pre>

* `[serial_no]` (for the minion) is obtained with `microbit_serial_number()`.
* `[build_no]` is a char representing the epidemic software version of the minion. The manager on the laptop will print
a warning if the minion's software version is out of date.
* `[friendly_id]` is the id in the 0..99 range, looked up from serials.csv on the laptop, and added if necessary and possible.
* `[master_ser]` is the master micro:bit's serial number, which it sends to the minion. (Recall that an epidemic is identified by
the master's serial number, and the epidemic id.
* `[master_time]` is the master micro:bit's current time, measured in milliseconds since the master received the parameters from the laptop.
* `[epid]`, `[R0]`, `[rtype]`, `[poimin]`, `[poimax]`, `[rpower]`, and `[exposure]` are the parameters the master previously received - see above.
* The message size so far is 27 bytes. Anecdotally we discovered 28-bytes is the maximum radio message size; above this, and the last bytes
are ignored. Therefore, the last three flags are combined into one byte; `[bcombine] = [btrans] + (4 * [brec]) + (16 * [icons])`.

### Seeding the epidemic

An infection is seeded from the laptop, which sends via serial to the master a message containing the victim id, and an indication 
of how the victim should make their contacts. The master then broadcasts this on the radio (REGISTERED group), and the susceptible
victim (who by definition is also listening on the REGISTERD group) hears the message, matches
their id with the id in the message, and acts accordingly.

<pre>
 ----------              |------------|     ----------  radio  |----------------------------|         ----------
 | Laptop |   serial     | 4[id],     |     | master |  bcast  | [SEED_MINION_MSG] : char   |         | minion |
 |        |------------->| [forcer],# |---->|        |- - - - >| [master_serial]   : int    |- - - - >|        |
 ----------              |------------|     ----------   on    | [epid]            : short  |         ----------
                                                         REG   | [id]              : short  |
                                                        group  | [n_contacts]      : char   |
                                                               |----------------------------|

</pre>

* `[id]` Refers to the friendly minion id (0.99) throughout
* `[master_serial]` and `[epid`] are serial number of the master, and the epidemic id as usual.
* `[n_contacts]` is either 0, meaning use the epidemic parameters to decide, or non-zero forces the victim to attemp a set number of contacts.
* On receiving and matching this message, the minion becomes infectious, 

### Infection

When a minion is infected, it broadcasts a message every second

### Recovery


### Reset the epidemic

Here, the laptop sends a message to the master, telling it to reset the micro:bits and
start another epidemic. The master broadcasts this to the micro:bits, and the micro:bits re-broadcast the message once, before
reseting, and returning to the _UNREGISTERED_ radio group. They start broadcasting their willingness to be part of the next
epidemic game.

<pre>                                                                                       repeat message once
                                                                                      | - - - - - - < - - - - - - - - - -|
 ----------              |-----|     ----------  radio   |-----------------------|    |               radio  |-----------------------|
 | Laptop |   serial     | 5#  |     | master |  bcast   | [RESET_MSG] : char    |    |   ----------  bcast  | [RESET_MSG] : char    |
 |        |------------->|     |---->|        |- - - - ->| [master_serial] : int |- - - ->| minion |- - - - >| [master_serial] : int |
 ----------              |-----|     ----------   REG    |-----------------------|        ----------   REG   |-----------------------|
                                                 group                                                group
</pre>

* As before, `[master_serial`] is the master's serial number, which identifies the epidemic; but this time, the RESET message will
be received also by any stragglers from a previous epidemic id, and they too will reset and be able to join the next game.

### Power-off the micro:bits.

Here, the laptop sends a message to the master, telling it to power-off the micro:bits. The master broadcasts this to the micro:bits, and the micro:bits re-broadcast the message.


<pre>                                                                                       repeat message once
                                                                                      | - - - - - - < - - - - - - - - - -|
 ----------              |-----|     ----------  radio   |-----------------------|    |               radio  |-----------------------|
 | Laptop |   serial     | 6#  |     | master |  bcast   | [POWEROFF_MSG] : char |    |   ----------  bcast  | [RESET_MSG] : char    |
 |        |------------->|     |---->|        |- - - - ->| [master_serial] : int |- - - ->| minion |- - - - >| [master_serial] : int |
 ----------              |-----|     ----------   REG    |-----------------------|        ----------   REG   |-----------------------|
                                                 group                                                group
</pre>

* As before, `[master_serial`] is the master's serial number, which identifies the epidemic; but this time, the POWER-OFF message will
be received also by any stragglers from a previous epidemic id, and they too will go to sleep, until manually restartd.
