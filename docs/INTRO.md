# public-events-microbit-epdemic: Introduction

This project is an epidemiology-themed game, in which each
player is a human wearing a micro:bit computer, and one game-master
administers the game. The game is loosely based on the well-known
S-I-R epidemiology models, where a person is either Susceptible to
an infection, Infected (and/or Infectious) with that diease, or they
have Recovered (or are Removed) from the game.

All players begin as *Susceptible* to an infection. The game-master
manually seeds an epidemic, causing one or more players to become
*Infected*. After this, the micro:bits spread the disease to each
other by proximity, using their simple bluetooth radio.

Live analysis of the epidemic can be displayed as the game 
progreses, including epicurves and network graphs. Each micro:bit
is labelled with a number so that the human can see where they fit
on the network graph; who has infected them, and who they
infected.

Each game setup by the game-master has various parameters to 
describe the range of the infection, the required exposure time that
causes an infection, and the number of victims a new infection might
cause. Other variations of the game can include prizes for the best
super-spreader, or the longest survivor, and a further twist involves
one of the micro:bits becoming a doctor who can cure other infected
micro:bits.