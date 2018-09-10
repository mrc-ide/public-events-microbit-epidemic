'''
epi_lang.py is part of the micro:bit epidemic project.
It contains all text elements of the management GUI,
such that in the future, language translations will
be convenient to create.

The MIT License (MIT)

Copyright (c) 2018 Wes Hinsley
MRC Centre for Global Infectious Disease Analysis
Department of Infectious Disease Epidemiology
Imperial College London

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

Created on 6 Jun 2018

@author: Wes Hinsley
'''

class EpiLang:

    title = "Epidemic on micro:bit"

    instructions_1 = ("Attach the Master micro:bit to a USB port, "
                      "select the com-port, and click the "
                      "Set Master Micro:Bit button")

    instructions_2 = ("Set the parameters for the epidemic. Then "
                      "click the Send Parameters button. See online "
                      "docs for parameter information")

    instructions_3 = ("Wait for green minions above, then click on a susceptible, or the"
                      "random button. Then click the Seed Epidemic button.")

    too_slow = "Too slow"
    no_longer_susc = "That host is no longer susceptible."
    end_epi = "End Epidemic"
    reset_or_power_off = "Type RESET for a new epidemic, or POWEROFF to kill the minions"
    error = "Error"
    mb_master_err = "Micro:Bit master is absent, wrong version, or needs restarting"
    err_r0 = "R0 must be numerical (e.g. 2.4)"
    epi_id_err = "Epidemic ID must be an integer"
    dup_pset = "Another parameter set has that name."
    exit = "Exit"
    really_exit = "Really exit?"
    set_master_mb = "Set Master Micro:Bit"
    send_params = "Send Parameters"
    seed_epi = "Seed Epidemic"
    reset_epi = "Reset Epidemic"
    current_task = "Current Task"
    using_port = "Using port:"
    rescan = "Rescan"
    software = "Software:"
    serial_no = "Serial No:"
    serial_error = "Can't open serial port. Already in use?"
    mb_ood = "Warning: micro:bit {} ({}) build {} is out of date. Update to {}"
    serial_lookup_err = "Warning: No space in serials.csv file for micro:bit serial no. {}"
    unrecog_serial = "Unrecognised serial device"
    mb_version = "micro:bit version:"
    epi_id = "Epidemic ID:"
    saved_params = "Saved Params:"
    save = "Save"
    delete = "Delete"
    delete_conf = "Really delete parameter set?"
    save_as = "Save As"
    save_first = "Save First"
    save_first_q = "Do you want to save changes to parameter set?"
    R0 = "R0:"
    R_type = "R type"
    poi_min = "Poi minimum"
    poi_max = "Poi maximum"
    transmit_range = "Transmit Range:"
    exposure = "Exposure (s):"
    transmit_button = "Transmit button"
    receive_button = "Receive button"
    icon_set = "Icon set"  
    constant = "Constant"
    poisson = "Poisson"
    name = "Name"
    name_pset = "Name of new parameter set?"