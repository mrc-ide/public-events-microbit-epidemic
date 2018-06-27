'''
Created on 6 Jun 2018

@author: Wes Hinsley
'''

from threading import Thread
import time
import serial.tools.list_ports
from serial.serialutil import SerialException

class EpiSerial:
    
    MSG_IN_VERSION = 'VER:'
    MSG_IN_REGISTER = 'REG:'
    MSG_IN_PARAM = 'PAR:'
    MSG_IN_INF = 'INF:'
    MSG_IN_RECOV = 'REC:'
    
    MSG_IDENTIFY_YOURSELF = '1\r\n'
    MSG_OUT_PARAMS = '2'
    MSG_SEED_EPI = '3'
    
    MICROBIT_PID = 516
    MICROBIT_VID = 3368
    
    
    # Loop to read from the port  while there is data to be
    # read. This executes continually in its own thread, and sleeps
    # for 1/10 s when there is no data to process.
      
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
                        self.handle_serial_data(reading)
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
        if (self.serial_port!=0):
            self.serial_port.close
            self.serial_port = 0
            
        try:
            self.serial_port = serial.Serial(port, 115200, timeout=0)
            self.serial_port.write(self.MSG_IDENTIFY_YOURSELF)
        
        except SerialException:
            self.gui_link.sv_software.set("Can't open serial port. Already in use?")
        
    # Process incoming serial data    
        
    def handle_serial_data(self, data):
        print data
        if (len(data)>4):
            if (data[0:4]==self.MSG_IN_VERSION):
                self.gui_link.sv_software.set(data.split(":")[1])
                self.gui_link.sv_serialno.set(data.split(":")[2])
                self.gui_link.sv_mbitver.set(data.split(":")[3])
            
            elif (data[0:4]==self.MSG_IN_REGISTER):
                self.gui_link.set_minion_status(data.split(":")[2], 'green')
                
            elif (data[0:4]==self.MSG_IN_PARAM):
                print "PARAM ACK"
            
            elif (data[0:4]==self.MSG_IN_INF):
                self.gui_link.set_minion_status(data.split(":")[2], 'red')
                #Do more stuff about infection
                
            elif (data[0:4]==self.MSG_IN_RECOV):
                self.gui_link.set_minion_statue(data.split(":")[2], 'blue')
                #Do more stuff about recovery
                
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
        self.serial_port.write(self.MSG_OUT_PARAMS+
                               self.gui_link.sv_epidno.get() + "\t" +
                               self.gui_link.sv_r0.get() + "\t" +
                               str(self.gui_link.cb_rtype.current()) + "\t" +
                               self.gui_link.cb_rpower.get() + "\t" +
                               self.gui_link.cb_exposure.get() + "\t\r\n")
        
    
    # Send seeding information to master, who forwards it by radio to minion.
    def seed_epidemic(self):
        forcer = 0
        if (self.gui_link.iv_forcer.get()==1):
            forcer = 1 + self.gui_link.cb_forcer.current()
            
        self.serial_port.write(self.MSG_SEED_EPI+
                               self.gui_link.sv_seedid.get() +"\t" +
                               str(forcer) + "\t\r\n")
        
    # Initialise serial port listener thread
    
    def __init__(self):
        self.gui = 0
        self.serial_port = 0
        self.serial_port_thread = Thread(target = self.read_from_port)
        self.serial_port_thread.setDaemon(True)
        self.serial_port_thread.start()

