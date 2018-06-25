#!/usr/bin/python
'''
@author: Wes Hinsley

This is the GUI code that controls the micro:bit master of
the epidemic.
 
'''

from epi_gui import EpiGui
from epi_serial import EpiSerial

EpiGui(EpiSerial())

