@echo off
if "%3"==""  goto args_count_wrong

::disable eeprom safe
tools\avrdude.exe -pm328p -cusbasp -Ulfuse:w:0xff:m -Uhfuse:w:0xdf:m -Uefuse:w:0x04:m

::erase chip and upload FW
tools\avrdude.exe -pm328p -cusbasp -e

::set chip ID and input polarization in flash
tools\avrdude.exe -pm328p -cusbasp -Ueeprom:w:%1,%2,%3,%4:m

::enable eeprom safe
tools\avrdude.exe -pm328p -cusbasp -U lfuse:w:0xff:m -U hfuse:w:0xd7:m -U efuse:w:0x04:m

::upload flash
tools\avrdude.exe -pm328p -cusbasp -Uflash:w:HomeLightControl.ino.eightanaloginputs.hex:i 

goto end

:args_count_wrong
echo usage: format.bat id_of_node(0x for hex) polarity_inputs0-7  polarity_inputs8-16 polarity_outputs0-7 (0 - normal active low, 1 reversed active high priority)


:end

