'''
Created on 6 Jun 2018

@author: Wes Hinsley
'''

from tkinter import Button, Label, Tk, PhotoImage, Entry, StringVar, IntVar, Checkbutton, END
import tkMessageBox, tkSimpleDialog
import os
import tkFont
from epi_lang import EpiLang
from ttk import Combobox
from tkinter.scrolledtext import ScrolledText

class EpiGui:
    
    CURRENT_EPI_ID = -1
    
    CHOOSE_MASTER = 1
    SET_PARAMS = 2
    SEED_EPIDEMIC = 3
    
    BUTTON_COL = 16
    TOP = 2
    LEFT = 12
                
    # The defaults are:
    # (1) The ID of an epidemic (so we can distinguish one epidemic from another)
    # (2) Previously used sets of parameters for an epidemic.
    #
    # I store these in a defaults.ini file, with a CSV-like format:-
    # First line is always:    epid,1    -  
    # All other lines:         "Param Set",r0_value,"r_type",transmit_power,exposure_time
    #              eg:         "Default",2,"poisson",4,10
    
    # Here I load all the parameter sets...
      
    def load_defaults(self):
        with open("defaults.ini","r") as ini_file:
            for line in ini_file:
                line = line.replace('\r','')
                line = line.replace('\n','')
                s = line.split(",")
                
                if ((len(s) == 2) & (s[0] == 'epid')):
                    self.CURRENT_EPI_ID = int(s[1])+1
                
                elif (len(s)==5):
                    self.paramsets.append(s[0].replace('"', ''))
                    self.p_r0.append(s[1])
                    self.p_rtype.append(s[2].replace('"', ''))
                    self.p_rpower.append(s[3])
                    self.p_exposure.append(str(s[4]))
        
        self.cb_paramset['values']=self.paramsets
        self.cb_paramset.current(0)
        self.remember_paramset = 0
        self.select_parameterset()
    
    # Save the defaults.ini file with new data. 
    
    def save_defaults(self):
        out_file = open("defaults.ini", "w")
        out_file.write("epid,{0}\n".format(self.CURRENT_EPI_ID))
        for i in range(0, len(self.paramsets)):
            out_file.write('"{0}",{1},"{2}",{3},{4}\n'.format(
                self.paramsets[i], 
                self.p_r0[i], 
                self.p_rtype[i], 
                self.p_rpower[i], 
                self.p_exposure[i]))
                           
        out_file.close()
        
    # Choose a particular parameter set from the list...
    
    def change_paramset(self, event):
        i = self.cb_paramset.current()
        if (self.remember_paramset != i):
            resp = False
            if (self.b_save_pset['state'] == 'active'):
                resp = tkMessageBox.askyesnocancel("Save First", "Do you want to save changes to parameter set?")
            if (resp == True):
                self.save_params(index = self.remember_paramset)
                resp = False
            
            # Either... there were no changes, or, there were changes we didn't want to save,
            # Or, there were changes we saved and wanted to continue... resp == False for all 3.
            
            if (resp == False):
                self.select_parameterset()
                self.remember_paramset = i
            
    def select_parameterset(self):
        i = self.cb_paramset.current()
        self.sv_r0.set(self.p_r0[i])
        self.cb_rtype.current(self.cb_rtype['values'].index(self.p_rtype[i]))
        self.cb_rpower.current(self.cb_rpower['values'].index(self.p_rpower[i]))
        self.cb_exposure.current(self.cb_exposure['values'].index(self.p_exposure[i]))
        
        if (len(self.paramsets) > 1):
            self.b_del_pset['state'] = 'active'
        else:
            self.b_del_pset['state'] = 'disabled'
        self.b_save_pset['state'] = 'disabled'
        
    # Delete a parameter set
    
    def del_params(self):
        if tkMessageBox.askokcancel("Delete", "Really delete parameter set?"):
            i = self.cb_paramset.current()
            del self.p_r0.remove[i]
            del self.p_rtype[i]
            del self.p_rpower[i]
            del self.p_exposure[i]
            del self.paramsets[i]
            self.b_save_pset['state'] = 'disabled'
            self.cb_paramset['values'] = self.paramsets
            self.cb_paramset.current(min(i, len(self.paramsets) - 1))
            self.save_defaults()
            self.select_parameterset()
            
    # Save changes to the parameter set structure in RAM.
    # And then write all parameter sets to the file.
    
    def validate_params(self):
        # Do some validation first.
        valid = True
        try:
            float(self.sv_r0.get())
        except:
            tkMessageBox.showerror("Error", "R0 must be numerical (e.g. 2.4)")
            valid = False
        
        try:
            int(self.sv_epidno.get())
        except:
            tkMessageBox.showerror("Error", "Epidemic ID must be an integer")
            valid = False
            
        return valid
    
    # Save parameter config...
    
    def save_params(self, index = -1):
        
        # Default: save the currently viewed parameter set (index=-1)
        # If this happened because of a "save unsaved changes", then index can specify
        # which parameter set we want to save.
        
        
        if self.validate_params():
            if (index==-1):
                index = self.cb_paramset.current()
            
            self.p_r0[index] = self.sv_r0.get()
            self.p_rtype[index] = self.cb_rtype.get()
            self.p_rpower[index] = self.cb_rpower.get()
            self.p_exposure[index] = self.cb_exposure.get()
            self.save_defaults()
            self.b_save_pset['state']='disabled'
        
    # Save current settings as a new parameter set
    
    def saveas_params(self):
        new_name = tkSimpleDialog.askstring("Name", "Name of new parameter set?")
        
        # Handle cancel / empty string
        
        if new_name is not None:
            new_name = new_name.strip()
            if (len(new_name) == 0):
                new_name = None
        
        # Handle duplicate name
                 
        if new_name is not None:
            try:
                self.paramsets.index(new_name)
                tkMessageBox.showerror("Error", "Another parameter set has that name.")
                new_name = None
            except:
                pass
        
        # Ok, save and select.
        if new_name is not None:
            self.p_exposure.append(self.cb_exposure.get())
            self.p_rpower.append(self.cb_rpower.get())
            self.p_rtype.append(self.cb_rtype.get())
            self.p_r0.append(self.sv_r0.get())
            self.paramsets.append(new_name)
            self.save_defaults()
            self.cb_paramset['values'] = self.paramsets
            self.cb_paramset.current(len(self.cb_paramset['values']) - 1)
            self.select_parameterset()
            self.b_save_pset['state'] = 'disabled'
        
    # Events where parameters are changed.
    
    def change_epidno(self, event):
        print("Change epid no")

    def set_params_unsaved(self, event):
        self.b_save_pset['state'] = 'active'
        
    # Exit confirmation dialogue
    
    def ask_quit(self):
        if tkMessageBox.askokcancel("Exit", "Really exit?"):
            self.window.destroy()
            
    
    def click_set_master(self):
        proceed = False
        if (self.sv_software.get() != 'Epi Master 1.0'):
            tkMessageBox.showerror("Error", "Micro:Bit master is not on that serial port, or needs restarting")
        else:
            self.set_task_text(self.lang.instructions_2)
            proceed = True
            
        if (proceed):
            self.b_setMaster['state']='disabled'
            self.b_sendParams['state']='active'
            self.grid_forget_all([self.cb_masters, self.b_rescan, self.l_port,
            self.l_software, self.l_software2,
            self.l_serialno, self.l_serialno2,
            self.l_mbitver, self.l_mbitver2])
            self.l_epidno.grid(column = self.LEFT, row = self.TOP, sticky = "E")
            self.e_epidno.grid(column = 1 + self.LEFT, row = self.TOP, sticky = "W")
            self.l_paramset.grid(column = self.LEFT, row = 2 + self.TOP, sticky = "E")
            self.cb_paramset.grid(column = 1 + self.LEFT, row = 2 + self.TOP, sticky = "W")
            self.b_save_pset.grid(column = 2 + self.LEFT, row = 2 + self.TOP, sticky = "W")
            self.b_del_pset.grid(column = 3 + self.LEFT, row = 2 + self.TOP, sticky = "W")
            self.b_saveas_pset.grid(column = 4 + self.LEFT, row = 2 + self.TOP, sticky = "W")
            
            self.l_r0.grid(column = self.LEFT, row = 3 + self.TOP, sticky = "E")
            self.e_r0.grid(column = 1 + self.LEFT, row = 3 + self.TOP, sticky = "W")
            self.l_rtype.grid(column = self.LEFT, row = 4 + self.TOP, sticky = "E")
            self.cb_rtype.grid(column = 1 + self.LEFT, row = 4 + self.TOP, sticky = "W")
            self.l_rpower.grid(column = self.LEFT, row = 5 + self.TOP, sticky = "E")
            self.cb_rpower.grid(column = 1 + self.LEFT, row = 5 + self.TOP, sticky = "W")
            self.l_exposure.grid(column = self.LEFT, row = 6 + self.TOP, sticky = "E")
            self.cb_exposure.grid(column = 1 + self.LEFT, row = 6 + self.TOP, sticky = "E")

    def click_send_params(self):
        self.save_defaults()
        self.b_setMaster.grid_forget()
        self.b_sendParams.grid_forget()
        self.b_seedEpidemic.grid(column = 0, row = 18, columnspan = 5)
        self.serial_link.send_params()
        self.set_task_text(self.lang.instructions_3)
        self.grid_forget_all([self.l_epidno, self.e_epidno, self.l_paramset, self.cb_paramset,
                self.b_save_pset, self.b_del_pset, self.b_saveas_pset, self.l_r0,
                self.e_r0, self.l_rtype, self.cb_rtype, self.l_rpower, self.cb_rpower,
                self.l_exposure, self.cb_exposure])
        self.l_seedid.grid(column = self.LEFT, row = self.TOP, sticky = "E")
        self.l_seedid2.grid(column = 1 + self.LEFT, row = self.TOP, sticky = "W")
        self.l_forcer.grid(column = self.LEFT, row = 1 + self.TOP, sticky = "E")
        self.tb_forcer.grid(column = 1 + self.LEFT, row = 1 + self.TOP, sticky = "W")
        self.l_ncons.grid(column = self.LEFT, row = 2 + self.TOP, sticky = "E")
        self.cb_forcer.grid(column = 1 + self.LEFT, row = 2 + self.TOP, sticky = "W")
        self.b_sendParams['state']='disabled'
        self.b_seedEpidemic['state']='active'
        

            
    def click_seed_epi(self):
        # TODO: Checking here
        
        self.serial_link.seed_epidemic()
        self.sv_seedid.set("")
    
    def click_minion(self, m_id):
        m_id = int(m_id)
        if (self.serial_link.serial_port != 0):
            if (self.minions[m_id % 10][m_id / 10]['bg']=='green'):
                self.sv_seedid.set(m_id)
            else:
                self.sv_seedid.set('')
            
    def set_minion_status(self, minion_id, status):
        m_id = int(minion_id)
        self.minions[m_id % 10][m_id / 10]['bg'] = status
        
    # Update the instructions box for what to do in this stage of the epidemic.
    
    def set_task_text(self, t):
        self.st_instruct['state'] = 'normal'
        self.st_instruct.delete('1.0', END)
        self.st_instruct.insert('insert', t)
        self.st_instruct['state'] = 'disabled'
        
    # Remove all the give components from the grid layout.
    
    def grid_forget_all(self, widgets):
        for widget in widgets:
            widget.grid_forget()

    def click_forcer(self):
        if (self.iv_forcer.get()==0):
            self.cb_forcer['state'] = 'disabled'
        else:
            self.cb_forcer['state'] = 'active'
        
    # Create the GUI. 
    
    def __init__(self, serial_link):
        
        # Basic definitions for the GUI components.
        
        self.window = Tk()
        self.lang = EpiLang()
        self.serial_link = serial_link
        self.serial_link.set_gui_link(self)
        
        # GUI elements present on all pages.
        
        self.minions =  [[0 for x in xrange(10)] for y in xrange(10)]
        
        for x in range(10):
            for y in range(10):
                n=((y * 10) + x)
                if (n < 10): n = "0" + str(n)
                self.minions[x][y] = Button(self.window, text = n, 
                    command = lambda n1 = n: self.click_minion(n1))
                self.minions[x][y].grid(column = x, row = y)

        self.b_setMaster = Button(self.window, text = 'Set Master Micro:Bit', command = self.click_set_master)
        self.b_sendParams = Button(self.window, text = 'Send Parameters', command = self.click_send_params)
        self.b_seedEpidemic = Button(self.window, text='Seed Epidemic', command = self.click_seed_epi)
        
        self.l_task = Label(self.window, text = "Current Task")
        self.st_font = tkFont.Font(family = "Calibri", size = 10)
        self.st_instruct = ScrolledText(self.window, width = 30, height = 5, font = self.st_font,
                                        wrap = 'word', state = 'disabled')

        self.b_sendParams['state'] = 'disabled'
        self.b_setMaster['state'] = 'active'
        
        self.b_setMaster.grid(column = 0, row = 18, columnspan = 5)
        self.b_sendParams.grid(column = 5, row = 18, columnspan = 5)
        self.l_task.grid(column = 0, row = 13, columnspan = 10)
        self.st_instruct.grid(column=0, row = 14, columnspan = 10)
        self.set_task_text(self.lang.instructions_1)
        
        # GUI elements for the Micro:Bit master selection page
        
        self.l_port = Label(self.window, text = "Using port:")
        self.cb_masters = Combobox(self.window, state = 'readonly')
        self.cb_masters.bind("<<ComboboxSelected>>", self.serial_link.get_master_info)
        self.cb_masters_double_click = 0
        self.b_rescan = Button(self.window, text = "Rescan", 
            command = self.serial_link.refresh_microbit_comports)
        self.l_software = Label(self.window, text = "Software:")
        self.sv_software = StringVar()
        self.sv_software.set("")
        self.l_software2 = Label(self.window, textvariable = self.sv_software)
        
        self.l_serialno = Label(self.window, text = "Serial No:")
        self.sv_serialno = StringVar()
        self.l_serialno2 = Label(self.window, textvariable = self.sv_serialno)
        self.l_mbitver = Label(self.window, text = "micro:bit version:")
        self.sv_mbitver = StringVar()
        self.l_mbitver2 = Label(self.window, textvariable = self.sv_mbitver)
                
        # GUI elements for the parameter settings page
                
        self.l_epidno = Label(self.window, text = "Epidemic ID:")
        self.sv_epidno = StringVar()
        self.e_epidno = Entry(self.window, textvariable = self.sv_epidno)
        
        self.l_paramset = Label(self.window, text = "Saved Params:")
        self.cb_paramset = Combobox(self.window, state = 'readonly')
        self.b_save_pset = Button(self.window, text = "Save", command = self.save_params)
        self.b_del_pset = Button(self.window, text = "Delete", command = self.del_params)
        self.b_saveas_pset = Button(self.window, text = "Save As", command = self.saveas_params)
        
        self.l_r0 = Label(self.window, text = "R0:")
        self.sv_r0 = StringVar()
        self.e_r0 = Entry(self.window, textvariable = self.sv_r0)
        self.l_rtype = Label(self.window, text = "R type")
        self.cb_rtype = Combobox(self.window, state = 'readonly')
        self.l_rpower = Label(self.window, text = "Transmit Range:")
        self.cb_rpower = Combobox(self.window, state = 'readonly')
        self.l_exposure = Label(self.window, text = "Exposure (s):")
        self.cb_exposure = Combobox(self.window, state = 'readonly')
        self.cb_rtype['values'] = ['Constant', 'Poisson']
        self.cb_rpower['values'] = [0, 1, 2, 3, 4, 5, 6, 7]
        self.cb_exposure['values'] = [1, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120]

        self.cb_paramset.bind("<<ComboboxSelected>>", self.change_paramset)
        self.remember_paramset = -1
        self.cb_rtype.bind("<<ComboboxSelected>>", self.set_params_unsaved)
        self.cb_rpower.bind("<<ComboboxSelected>>", self.set_params_unsaved)
        self.cb_exposure.bind("<<ComboboxSelected>>", self.set_params_unsaved)
        self.e_r0.bind("<Key>", self.set_params_unsaved)
        self.e_epidno.bind("<Key>", self.change_epidno)

        # Gui Elements for the Seeding Page

        self.l_seedid = Label(self.window, text = "Victim:")
        self.sv_seedid = StringVar()
        self.l_seedid2 = Label(self.window, textvariable = self.sv_seedid)
        self.l_forcer = Label(self.window, text = "Force contacts")
        self.iv_forcer = IntVar()
        self.iv_forcer.set(1)
        self.tb_forcer = Checkbutton(self.window, text="", variable = self.iv_forcer, command=self.click_forcer)
        self.l_ncons = Label(self.window, text = "No. contacts")
        self.cb_forcer = Combobox(self.window, state = 'readonly')

        self.cb_forcer['values'] = [1,2,3,4,5,6]
        self.cb_forcer.current(3)

        # Load the previously-saved configurations

        self.paramsets = []
        self.p_r0 = []
        self.p_rtype = []
        self.p_rpower = []
        self.p_exposure = []
        
        self.load_defaults()
        self.sv_epidno.set(self.CURRENT_EPI_ID)
        
        # Window specific stuff. Set what happens on exit
        # Set the size, title, and application icon for the window.
        
        self.window.protocol("WM_DELETE_WINDOW", self.ask_quit)
        self.window.title("Epidemic on micro:bit")
        self.window.geometry("640x480")
        imgicon = PhotoImage(file = os.path.join('microepi.gif'))
        self.window.tk.call('wm', 'iconphoto', self.window._w, imgicon)
        
        # Show first interface
        
        self.l_port.grid(column = self.LEFT, row = self.TOP)
        self.cb_masters.grid(column = 1 + self.LEFT, row = self.TOP)
        self.b_rescan.grid(column = 2 + self.LEFT, row = self.TOP)
        self.l_software.grid(column = self.LEFT, row = 1 + self.TOP, sticky = "E")
        self.l_software2.grid(column = 1 + self.LEFT, row = 1 + self.TOP, sticky = "W")
        self.l_serialno.grid(column = self.LEFT, row = 2 + self.TOP, sticky = "E")
        self.l_serialno2.grid(column = 1 + self.LEFT, row = 2 + self.TOP, sticky = "W")
        self.l_mbitver.grid(column = self.LEFT, row = 3 + self.TOP, sticky = "E")
        self.l_mbitver2.grid(column = 1 + self.LEFT, row = 3 + self.TOP, sticky = "W")

        self.serial_link.refresh_microbit_comports()
        self.window.mainloop()

    