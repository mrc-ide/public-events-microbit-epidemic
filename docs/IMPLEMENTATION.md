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

### Serial

Messages over serial are supposed to be sent from buffers when a new-line is sent. However, spurious 
new-lines seemed to be commonly inserted. The best solution seemed to be to send serial messages ending with 
a new-line, but also to include our own end-of-message character: _#_. New lines are then used purely to trigger 
a buffer flush and send; they are stripped from incoming messages, and buffered until a _#_ is read,
which indicates the message has been received completely. Any non-newline left-over after the _#_ is buffered as the start of the
next message.

### Radio

* All messages over radio are broadcasts, rather than point-to-point messages. The links between masters and minions in 
the message diagrams below are therefore somewhat implied; all micro:bits in range hear the message, and at least partially
unpack it, but whether they do anything more depends on the message content.

* Messages over radio are broadcast with a customisable transmit power. Generally, we transmit at maximum power, except for
the infection broadcast message, which we sent at a power level defined as an epidemic parameter.

* The micro:bits listen, and broadcast on a certain group - a bit like a channel; they can be members of one group at a time.
Groups are designed to separate messages so that they are only received by those interested in them. We use two groups, one 
for _REGISTERED_ micro:bits, and another for _UNREGISTERED_. Thus messages requesting registration are ignored all the while
the epidemic is in progress, and are only noticed in the registration window at the start of the epidemic.

* Messages over radio appear to be limited to 28 bytes in length - not the 32 that the micro:bit documentation reports. 

### Identification of micro:bit

Here, the laptop sends a requst to a connected micro:bit, asking it to identify itself in various ways.

<pre>

 ----------   serial     |---------|          ----------   
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
 |        |  bcast   | [REG_MSG]   : char  |  bcast   |        |       | REG:[serial_no]: | serial | Laptop |
 |        |   on     | [serial_no] : int   |   on     |        |       | [build_no]#      |        |        |
 |        |- - - - ->| [build_no]  : char  |- - - - ->| master |------>|                  |------->|        |
 |        |  UNREG   |                     |  UNREG   |        |       |------------------|        |        |
 |        |  group   |---------------------|  group   |        |                                   |        |
 | Minion |                                           |        |       |------------------|        |        |
 |        |        |-----------------------|          |        |<------| 2[serial_no],    |<-------|        |
 |        |        | [REG_ACK_MSG] : char  |<- - - - -|        |       | [friendly_id],#  |        ----------
 |        |        | [serial_no]   : int   |          ----------       |------------------|
 |        |        | [friendly_id] : short |
 |        |        | [master_ser]  : int   |
 |        |<- - - -| [master_time0]: int   |
 |        |        | [epid]        : short |
 |        |        | [R0]          : float |
 ----------        | [rtype]       : char  |
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
* `[master_time0]` is the master micro:bit's time, measured in milliseconds since the master received the parameters from the laptop.
On receiving this message, the minion also notes their current system time, hence future timestamps can be reported as elapsed epidemic time 
by calculating `uBit.systemTime() - my_time0) + master_time0`.

* `[epid]`, `[R0]`, `[rtype]`, `[poimin]`, `[poimax]`, `[rpower]`, and `[exposure]` are the parameters the master previously received - see above.
* The message size so far is 27 bytes. Anecdotally we discovered 28-bytes is the maximum radio message size; above this, and the last bytes
are ignored. Therefore, the last three flags are combined into one byte; `[bcombine] = [btrans] + (4 * [brec]) + (16 * [icons])`.

### Seeding the epidemic

An infection is seeded from the laptop, which sends via serial to the master a message containing the victim id, and an indication 
of how the victim should make their contacts. The master then broadcasts this on the radio (_REGISTERED_ group), and the susceptible
victim (who by definition is also listening on the _REGISTERED_ group) hears the message, matches
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

When a minion is infected, it broadcasts a message every second to anyone who listens, at the transmision power set in the parameters offering to contact (and if susceptible, infect) them. 
Minions keep a count of the number of infection offers they have received from each potential victim, and once the exposure threshold is reached, 
the "contact" replies to the potential infector accepting their offer. It is very possible that the infector will receive more replies from 
potential victims than it wants, so therefore it replies to confirm contact with each victim. Having made all its contacts, the infector recovers,
and the contacts it made who were susceptible becoming infected.

<pre>

Contact Negotiation:

           power:rpower                                            power:MAX
 ----------   radio      |------------------------|      ----------  radio   |------------------------|
 | Minion |   bcast      | [INF_BCAST_MSG] : char |      | Minion |  bcast   | [INF_CAND_MSG] : char  |
 |        |- - - - - - ->| [master_serial] : int  |- - ->|        |- - - - ->| [master_serial] : int  |
 |        |    REG       | [epid]          : short|      ----------   REG    | [epid]          : short|
 |        |   group      | [inf_id]        : short|                  group   | [inf_id]        : short|
 |        |              |------------------------|                          | [victim_id]     : short|
 |        |<- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|                        |
 ----------                                                                  |------------------------|

Contact Confirmation:

            power:MAX
 ----------   radio      |------------------------|      ----------
 | Minion |   bcast      | [INF_CONF_MSG]  : char |      | Minion |
 |        |- - - - - - ->| [master_serial] : int  |- - ->|        |
 |        |    REG       | [epid]          : short|      ----------
 |        |   group      | [inf_id]        : short|
 |        |              | [victim_id]     : short|
 |        |              |------------------------|
 ----------

</pre>

* `[master_serial]` and `[epid]` are the master serial number, and epidemic number as usual.
* `[inf_id]` is the friendly id of the infector (0..99), and `[victim_id]` the friendly id of a potential contact.

### Reporting Infection Status

When a susceptible minion becomes infected, it broadcasts the relevant data to the master every second until the master 
replies with an acknowledgement. The master forwards the infection data over serial to the laptop; a CSV file gets
appended to, and the Slideshow visualisations are updated.

<pre>

 ----------   radio      |------------------------|      ----------
 | Minion |   bcast      | [REP_INF_MSG]   : char |      |        |
 |        |- - - - - - ->| [master_serial] : int  |- - ->|        |
 | (new   |    REG       | [epid]          : short|      |        |        |---------------|
 | victim)|   group      | [inf_id]        : short|      |        | serial | INF:[inf_id]: |     ----------
 |        |              | [victim_id]     : short|      |        |------->| [victim_id]:  |---->| Laptop |
 |        |              | [inf_time]      : int  |      | Master |        | [inf_time]:   |     ----------
 |        |              | [n_contacts]    : char |      |        |        | [n_contacts]# |
 |        |              |------------------------|      |        |        |---------------|
 |        |                                              |        |
 |        |           |---------------------------|      |        |
 |        |           | [CONF_REP_INF_MSG] : char |      |        |
 |        |           | [master_serial]    : int  |<- - -|        |
 |        |< - - - - -| [epid]             : short|      ----------
 ----------           | [victim_id]        : short|
                      |---------------------------|
</pre>

### Recovery

When an infected minion has made all its contacts, it recovers: it stops broadcasting offers to
infect other minions, and ignores any further victim offers that it receives. It broadcasts
its recovery to the master, until it receives an acknowledgement, and the master forwards it
to the laptop for updating the CSV file, and the Slideshow visualisations.

<pre>

 ----------   radio      |------------------------|      ----------
 | Minion |   bcast      | [REP_RECOV_MSG] : char |      |        |
 |        |- - - - - - ->| [master_serial] : int  |- - ->|        |
 |        |    REG       | [epid]          : short|      |        |        |---------------|
 |        |   group      | [victim_id]     : short|      |        | serial | REC:          |     ----------
 |        |              | [recov_time]    : int  |      |        |------->| [victim_id]:  |---->| Laptop |
 |        |              |------------------------|      | Master |        | [recov_time]# |     ----------
 |        |                                              |        |        |---------------|
 |        |         |-----------------------------|      |        |
 |        |         | [CONF_REP_RECOV_MSG] : char |      |        |
 |        |         | [master_serial]      : int  |<- - -|        |
 |        |< - - - -| [epid]               : short|      ----------
 ----------         | [victim_id]          : short|
                    |-----------------------------|
</pre>

* `[master_serial]` and `[epid]` are the master serial number, and epidemic number as usual.
* `[victim_id]` is the friendly id of the victim who has recovered. (0..99).
* `[recov_time]` is the time of recovery, in milliseconds since the master received the epidemic parameters. Calculated by `uBit.systemTime() - my_time0) + master_time0`.

### Reset the epidemic

Here, the laptop sends a message to the master, telling it to reset the micro:bits and
start another epidemic. The master broadcasts this to the micro:bits, and the micro:bits re-broadcast the message once, before
reseting, and returning to the _UNREGISTERED_ radio group. They start broadcasting their willingness to be part of the next
epidemic game.

<pre>

 ----------              |-----|     ----------  radio   |-----------------------|
 | Laptop |   serial     | 5#  |     | master |  bcast   | [RESET_MSG] : char    |        ----------
 |        |------------->|     |---->|        |- - - - ->| [master_serial] : int |- - - ->|        |
 ----------              |-----|     ----------   REG    |-----------------------|        |        |
                                                 group                                    |        |
                                                  - - - - - - - - - - - - - - - - - - - ->| minion |
                                                  |     |-----------------------| radio   |        |
                                                  |     | [RESET_MSG] : char    | bcast   |        |
                                                   - - -| [master_serial] : int |<-once --|        |
                                                        |-----------------------|  REG    ----------
                                                                                  group
</pre>

* As before, `[master_serial`] is the master's serial number, which identifies the epidemic; but this time, the RESET message will
be received also by any stragglers from a previous epidemic id, and they too will reset and be able to join the next game.

### Power-off the micro:bits.

Here, the laptop sends a message to the master, telling it to power-off the micro:bits. The master broadcasts this to the micro:bits, and the micro:bits re-broadcast the message.

<pre>

 ----------              |-----|     ----------  radio   |-----------------------|
 | Laptop |   serial     | 6#  |     | master |  bcast   | [POWEROFF_MSG] : char |        ----------
 |        |------------->|     |---->|        |- - - - ->| [master_serial] : int |- - - ->|        |
 ----------              |-----|     ----------   REG    |-----------------------|        |        |
                                                 group                                    |        |
                                                  - - - - - - - - - - - - - - - - - - - ->| minion |
                                                  |     |-----------------------| radio   |        |
                                                  |     | [POWEROFF_MSG] : char | bcast   |        |
                                                   - - -| [master_serial] : int |<-once --|        |
                                                        |-----------------------|  REG    ----------
                                                                                  group
</pre>

* As before, `[master_serial`] is the master's serial number, which identifies the epidemic; but this time, the POWER-OFF message will
be received also by any stragglers from a previous epidemic id, and they too will go to sleep, until manually restartd.
