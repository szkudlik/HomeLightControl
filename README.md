# HomeLightControl
A simple peer-peer communication system for smart home light control
Based on Arduino-Nano as nodes and Arduino mega 2560 with EtherShield as a controler
Communication is 1-wire based, using TLE8457 chip. 


Features:
 - a slow, but very reliable serial communication with collision detection mechanism
 - a peer to peer communication, the controller node is not required in normal operation
 - any button can control any light - each node keeps a list of actions in EEPROM and acts accordingly when a button has been hit
 - actions may be programmed in each node by a controller using the same serial bus
