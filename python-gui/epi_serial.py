'''
Created on 6 Jun 2018

@author: Wes Hinsley
'''

from threading import Thread
import time
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
    
    MICROBIT_PID = 516
    MICROBIT_VID = 3368
    
    input_buffer = ""
    latest_minion_buildno = '10'
    
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
            self.gui_link.sv_software.set("Can't open serial port. Already in use?")
        
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
                    print "Warning: micro:bit {} ({}) build {} is out of date. Update to {}".format(serialno, 
                        friendlyid, buildno, self.latest_minion_buildno)

                if (friendlyid == '-1'):
                    print "Warning: No space in serials.csv file for micro:bit serial no. {}".format(serialno)
                else:
                    msg = "{}{},{},#".format(self.MSG_REG, serialno, friendlyid)
                    self.serial_port.write(msg+"\n")
                    self.gui_link.set_minion_status(friendlyid, self.gui_link.STATUS_SUSCEPTIBLE)
                
            elif (data[0:4] == self.MSG_IN_INF):
                bits = data.split(":")
                self.gui_link.set_minion_status(bits[2], self.gui_link.STATUS_INFECTED)
                fn = self.gui_link.sv_serialno.get() + "_" + self.gui_link.sv_epidno.get() + ".csv"
                
                if (not os.path.isfile(fn)):
                    with open(fn, "w") as f:
                        f.write("event,time,infector,victim,ncons\n")
                        f.write("I,{},{},{},{}\n".format(bits[3], bits[1] ,bits[2], bits[4]))
                                                
                else:
                    with open(fn, "a") as f:
                        f.write("I,{},{},{},{}\n".format(bits[3], bits[1] ,bits[2], bits[4]))
                    
                print data
                
            elif (data[0:4] == self.MSG_IN_RECOV):
                bits = data.split(":")
                self.gui_link.set_minion_status(bits[1], self.gui_link.STATUS_RECOVERED)
                fn = self.gui_link.sv_serialno.get() + "_" + self.gui_link.sv_epidno.get() + ".csv"
                
                with open(fn, "a") as f:
                    f.write("R,{},-1,{},-1\n".format(bits[2], bits[1]))
                
                print data
                
            else:
                self.gui_link.sv_software.set("Unrecognised serial device")
                self.gui_link.sv_serialno.set("")
                self.gui_link.sv_mbitver.set("")
                
        else:
            self.gui_link.sv_software.set("Unrecognised serial device")
            self.gui_link.sv_serialno.set("")
            self.gui_link.sv_mbitver.set("")
            
    # Allow serial class to talk to gui
    
    def set_gui_link(self, gui_link):
        self.gui_link = gui_link
    
    
    # Send the parameters to the micro:bit master.
    def send_params(self):
        print str(self.gui_link.cb_rtype.current())
              
        msg = (self.MSG_OUT_PARAMS+
               self.gui_link.sv_epidno.get() + "," +
               self.gui_link.sv_r0.get() + "," +
               str(self.gui_link.cb_rtype.current()) + "," +
               self.gui_link.cb_poimin.get() + "," +
               self.gui_link.cb_poimax.get() + "," +
               self.gui_link.cb_rpower.get() + "," +
               self.gui_link.cb_exposure.get() + ",#")
              
        self.serial_port.write(msg+"\n")
    
    # Send seeding information to master, who forwards it by radio to minion.
    def seed_epidemic(self):
        forcer = 0
        if (self.gui_link.iv_forcer.get()==1):
            forcer = 1 + self.gui_link.cb_forcer.current()
        msg = self.MSG_SEED_EPI + self.gui_link.sv_seedid.get() + "," + str(forcer) + ",#"
        self.serial_port.write(msg+"\n")
        
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
