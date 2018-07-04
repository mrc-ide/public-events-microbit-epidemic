# public-events-microbit-epdemic: Usage - running an epidemic!

Having got the hardware built, and either compiled the code, or
got the binaries ready, these steps will get an epidemic up and
running.

* TO-DO - sort out/talk about serial number collection

### Prepare the micro:bits

* Attach the micro:bit you decide is the master, to the USB port on a laptop. Note the drive letter.
* copy microbit-binaries\master_1_2.hex to that drive, to flash the master firmware onto the micro:bit.
* Attach each minion micro:bit one by one. What fun. And copy microbit-binaries\minion_1_2.hex to each.
* To-DO - talk about firmware
* Connect the master via USB to the laptop, and if necessary, install the serial driver from [here](https://os.mbed.com/handbook/Windows-serial-configuration)
(for windows that is).

### Use the python GUI
From the root of this repo, in a command window. Assuming you have python installed...
```
cd python-gui
python epi_manager.py
```

There are instructions in the bottom left of each stage...
