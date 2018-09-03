'''
Created on 6 Jun 2018

@author: Wes Hinsley
'''

from threading import Thread
import time

try:
    import serial.tools.list_ports
except ImportError, e:
    import pip
    pip.main(['install', 'pySerial'])
    import serial.tools.list_ports

import re
import csv
import os.path

from serial.serialutil import SerialException

class EpiSerial:

    MSG_IN_VERSION = 'VER:'
    MSG_IN_REGISTER = 'REG:'
    MSG_IN_INF = 'INF:'
    MSG_IN_RECOV = 'REC:'
    MSG_IN_DEBUG = 'DEB:'

    MSG_IDENTIFY_YOURSELF = '1#'
    MSG_REG = '2'
    MSG_OUT_PARAMS = '3'
    MSG_SEED_EPI = '4'
    MSG_RESET_EPI = '5#'
    MSG_POWER_OFF = '6#'

    # Output Path is relative to bin/python-gui
    OUTPUT_PATH = '../../data/'

    MICROBIT_PID = 516
    MICROBIT_VID = 3368

    input_buffer = ""
    latest_minion_buildno = '13'

    current_epi_t0 = 0

    RECENT_TIME_S = 900

    def get_friendly_id(self, sid):
        result = '-1'
        for i in range(len(self.serials)):
            if (self.serials[i]['serial'] == sid):
                result = self.serials[i]['id']
                break

        # Serial not found - add to file if there are blanks...

        if (result=='-1'):
            for i in range(len(self.serials)):
                if (self.serials[i]['serial'] == ''):
                    self.serials[i]['serial'] = sid
                    result = str(i)
                    with open('serials.csv', 'w') as f:
                        f.write('serial,id\n')
                        for i in range(len(self.serials)):
                            s = "{},{}\n".format(self.serials[i]['serial'],self.serials[i]['id'])
                            f.write(s)

        return result

    # Loop to read from the port  while there is data to be
    # read. This executes continually in its own thread, and sleeps
    # for 1/10 s when there is no data to process.

    # Python appeared to intermittently insert newlines into mid-message,
    # in ways not apparent when using Putty. Therefore, all new-lines are
    # ignored, and all valid incoming messages must end with '#'

    def read_from_port(self):
        while True:
            did_work = False
            if (self.serial_port!=0):
                if (self.serial_port.port!=""):
                    try:
                        reading = self.serial_port.readline().decode()
                    except:
                        reading = ''
                    if (len(reading)!=0):
                        did_work = True
                        self.input_buffer = self.input_buffer + reading
                        self.input_buffer = re.sub('[\n]', '', self.input_buffer)
                        if (self.input_buffer.endswith('#')):
                            self.input_buffer = re.sub('[#]','', self.input_buffer)
                            self.handle_serial_data(self.input_buffer)
                            self.input_buffer = ''

            if (did_work == False):
                time.sleep(0.1)


    # Find all serial ports with device ID matching a micro:bot.
    # If a micro:bit serial port was previous selected, remember it and
    # reselect it (if still available) after rescan.

    # For reasons I don't yet understand, double-clicking the combo-box causes
    # the app to lock up totally. First and last line therefore force ignoring
    # the double-click.

    def refresh_microbit_comports(self):
        self.gui_link.cb_masters_double_click = self.gui_link.cb_masters_double_click + 1

        if (self.gui_link.cb_masters_double_click == 1):
            old_val = self.gui_link.cb_masters.get()
            microbits = []
            ports = serial.tools.list_ports.comports()
            for p in ports:
                if (p.pid == self.MICROBIT_PID) and (p.vid == self.MICROBIT_VID):
                    microbits.append(p.device)
            microbits.sort()

            self.gui_link.cb_masters['values'] = microbits

            try:
                reselect = microbits.index(old_val)
            except ValueError:
                reselect = 0

            if (len(microbits)>0):
                self.gui_link.cb_masters.current(reselect)
                self.get_master_info(self)

        self.gui_link.cb_masters_double_click = self.gui_link.cb_masters_double_click - 1


    # Send a message to the serial port asking the micro:bit to identify itself.

    def get_master_info(self, event):
        port = self.gui_link.cb_masters.get()
        if (self.serial_port != 0):
            self.serial_port.close()
            self.serial_port = 0

        try:
            self.serial_port = serial.Serial(port, 115200, timeout=1, xonxoff=True)
            self.serial_port.write(self.MSG_IDENTIFY_YOURSELF+"\n")

        except SerialException:
            self.gui_link.sv_software.set(self.gui_link.lang.serial_error)

    # Process incoming serial data

    def handle_serial_data(self, data):
        if (len(data)>4):

            if (data[0:4] == self.MSG_IN_DEBUG):
                print data

            if (data[0:4] == self.MSG_IN_VERSION):
                self.gui_link.sv_software.set(data.split(":")[1])
                self.gui_link.sv_serialno.set(data.split(":")[2])
                self.gui_link.sv_mbitver.set(data.split(":")[3])

            elif (data[0:4] == self.MSG_IN_REGISTER):
                serialno = data.split(":")[1]
                buildno = data.split(":")[2]
                friendlyid = self.get_friendly_id(serialno)
                if (buildno != self.latest_minion_buildno):
                    print self.gui_link.lang.mb_ood.format(serialno,
                        friendlyid, buildno, self.latest_minion_buildno)

                if (friendlyid == '-1'):
                    print self.gui_link.lang.serial_lookup_err.format(serialno)
                else:
                    msg = "{}{},{},#".format(self.MSG_REG, serialno, friendlyid)
                    self.serial_port.write(msg+"\n")
                    self.gui_link.set_minion_status(friendlyid, self.gui_link.STATUS_SUSCEPTIBLE)

            elif (data[0:4] == self.MSG_IN_INF):
                #Incoming is INF:ID:VICTIM:TIME:NCONTACTS
                bits = data.split(":")
                self.gui_link.set_minion_status(bits[2], self.gui_link.STATUS_INFECTED)
                fn = self.OUTPUT_PATH + self.gui_link.sv_serialno.get() + "_" + self.gui_link.sv_epidno.get() + ".csv"

                if (not os.path.isfile(fn)):
                    with open(fn, "w") as f:
                        f.write("Event,TimeH,Mins,Infectedby,Seeding,Recency,Category,ID,NoContacts\n")

                with open(fn, "a") as f:
                    inf_time_epoch = self.current_epi_t0 + (float(bits[3]) / 1000.0)
                    inf_time = time.gmtime(inf_time_epoch)
                    mins = inf_time.tm_min + (inf_time.tm_sec/60.0)
                    seeding = 'N'
                    if (bits[1] == '32767'):
                        seeding = 'S'
                        bits[1] = 'NA'
                    recency = 'Old'
                    if (time.time() - inf_time_epoch < self.RECENT_TIME_S):
                        recency = 'Recent'

                    f.write("I,{},{},{},{},{},{},{},{}\n".format(
                        inf_time.tm_hour, mins, bits[1], seeding, recency, 0, bits[2], bits[4]))

                print data

            elif (data[0:4] == self.MSG_IN_RECOV):
                bits = data.split(":")
                self.gui_link.set_minion_status(bits[1], self.gui_link.STATUS_RECOVERED)
                fn = self.OUTPUT_PATH + self.gui_link.sv_serialno.get() + "_" + self.gui_link.sv_epidno.get() + ".csv"
                rec_time_epoch = self.current_epi_t0 + (float(bits[2]) / 1000.0)
                rec_time = time.gmtime(rec_time_epoch)
                mins = rec_time.tm_min + (rec_time.tm_sec/60.0)
                with open(fn, "a") as f:
                    f.write("R,{},{},NA,NA,NA,NA,{},NA\n".format(
                        rec_time.tm_hour, mins, bits[1]))

                print data

            else:
                self.gui_link.sv_software.set(self.gui_link.lang.unrecog_serial)
                self.gui_link.sv_serialno.set("")
                self.gui_link.sv_mbitver.set("")

        else:
            self.gui_link.sv_software.set(self.gui_link.lang.unrecog_serial)
            self.gui_link.sv_serialno.set("")
            self.gui_link.sv_mbitver.set("")

    # Allow serial class to talk to gui

    def set_gui_link(self, gui_link):
        self.gui_link = gui_link

    def write_xml_params(self):
        fn = self.OUTPUT_PATH + self.gui_link.sv_serialno.get() + "_" + self.gui_link.sv_epidno.get() + ".xml"
        players = ""
        for x in range(10):
            for y in range(10):
                col = self.gui_link.minions[x][y]['bg']
                if ((col == self.gui_link.STATUS_SUSCEPTIBLE) or (col == self.gui_link.STATUS_INFECTED) or (col == self.gui_link.STATUS_RECOVERED)):
                    if (players != ""):
                        players = players + ","
                    players = players + str((y * 10) + x)

        msg = (self.gui_link.sv_epidno.get() + "," +
               self.gui_link.sv_r0.get() + "," +
               str(self.gui_link.cb_rtype.current()) + "," +
               self.gui_link.cb_poimin.get() + "," +
               self.gui_link.cb_poimax.get() + "," +
               self.gui_link.cb_rpower.get() + "," +
               self.gui_link.cb_exposure.get() + "," +
               str(self.gui_link.cb_btrans.current()) + "," +
               str(self.gui_link.cb_brec.current()) + "," +
               str(self.gui_link.cb_icons.current()))

        with open(fn, "w") as f:
            f.write("<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n")
            f.write("<meta>\n")
            f.write("  <params>" + msg + "</params>\n")
            f.write("  <time>" + str(self.current_epi_t0) + "</time>\n")
            f.write("  <players>" + str(players) + "</players>\n")
            f.write("  <game>" + str(self.gui_link.cb_paramset.get()) + "</game>\n")
            f.write("</meta>")

        # Also check that a stub .csv exists; Slideshow wants to load a CSV file.

        fn = self.OUTPUT_PATH + self.gui_link.sv_serialno.get() + "_" + self.gui_link.sv_epidno.get() + ".csv"
        if (not os.path.isfile(fn)):
            with open(fn, "w") as f:
                f.write("Event,TimeH,Mins,Infectedby,Seeding,Recency,Category,ID,NoContacts\n")

    # Send the parameters to the micro:bit master.
    def send_params(self):

        msg = (self.MSG_OUT_PARAMS+
               self.gui_link.sv_epidno.get() + "," +
               self.gui_link.sv_r0.get() + "," +
               str(self.gui_link.cb_rtype.current()) + "," +
               self.gui_link.cb_poimin.get() + "," +
               self.gui_link.cb_poimax.get() + "," +
               self.gui_link.cb_rpower.get() + "," +
               self.gui_link.cb_exposure.get() + "," +
               str(self.gui_link.cb_btrans.current()) + "," +
               str(self.gui_link.cb_brec.current()) + "," +
               str(self.gui_link.cb_icons.current()) + ",#")

        self.serial_port.write(msg+"\n")
        self.current_epi_t0 = time.time()

        # Also write a meta file for the viewer.
        # It won't have all the hosts yet, but it's better
        # if slideshow has something to show before seeding,
        # especially on Saviour game.

        self.write_xml_params()
        


    # Send seeding information to master, who forwards it by radio to minion.
    def seed_epidemic(self):
        forcer = 0
        if (self.gui_link.iv_forcer.get()==1):
            forcer = 1 + self.gui_link.cb_forcer.current()

        msg = self.MSG_SEED_EPI + self.gui_link.sv_seedid.get() + "," + str(forcer) + ",#"
        self.serial_port.write(msg+"\n")
        self.write_xml_params()

    def reset_epidemic(self):
        self.serial_port.write(self.MSG_RESET_EPI+"\n")

    def poweroff_minions(self):
        self.serial_port.write(self.MSG_POWER_OFF+"\n")

    # Initialise serial port listener thread

    def __init__(self):

        self.gui = 0
        self.serial_port = 0
        self.serial_port_thread = Thread(target = self.read_from_port)
        self.serial_port_thread.setDaemon(True)
        self.serial_port_thread.start()

        with open("serials.csv") as f:
            reader = csv.DictReader(f)
            self.serials = [r for r in reader]
