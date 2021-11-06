@echo off

::erase chip and upload FW
tools\avrdude.exe -pm328p -cusbasp -e

::upload flash
tools\avrdude.exe -pm328p -cusbasp -Uflash:w:HomeLightControl.ino.eightanaloginputs.hex:i 


