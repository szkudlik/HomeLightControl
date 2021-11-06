#ifndef NETWORK_H
#define NETWORK_H

#include "Eeprom.h"
#include <SPI.h>
#include <Ethernet.h>

class tNetwork
{
public:
 tNetwork() {}
 
 void init()
 {
    for(uint8_t i= 0; i < 4; i++) mIP[i] = EEPROM.read(EEPROM_IP+i);
    for(uint8_t i= 0; i < 4; i++) mNetmask[i] = EEPROM.read(EEPROM_IPMASK+i);
    for(uint8_t i= 0; i < 4; i++) mGateway[i] = EEPROM.read(EEPROM_GATEWAY+i);
    for(uint8_t i= 0; i < 4; i++) mDNS[i] = EEPROM.read(EEPROM_DNS+i);
    for(uint8_t i= 0; i < 6; i++) mMAC[i] = EEPROM.read(EEPROM_MAC+i);

    Ethernet.begin(mMAC, mIP, mDNS, mGateway, mNetmask);
    while (Ethernet.hardwareStatus() == EthernetNoHardware);
 }

 const IPAddress * GetIp() const { return &mIP; }
 const IPAddress * GetNetmask() const { return &mNetmask; }
 const IPAddress * GetGateway() const { return &mGateway; }
 const IPAddress * GetDNS() const { return &mDNS; }
 const uint8_t * GetMAC() const { return mMAC; }

 void SetIp(uint8_t * IP) 
 {
   for(uint8_t i= 0; i < 4; i++) mIP[i] = IP[i];
   Update();
 }
 
 void SetNetmask(uint8_t * Netmask)
 {
   for(uint8_t i= 0; i < 4; i++) mNetmask[i] = Netmask[i];
   Update();
 }

 void SetGateway(uint8_t * Gateway) 
 {
   for(uint8_t i= 0; i < 4; i++) mGateway[i] = Gateway[i];
   Update();
 }

 void SetDNS(uint8_t * DNS)
 {
   for(uint8_t i= 0; i < 4; i++) mDNS[i] = DNS[i];
   Update();
 }

 void SetMAC(uint8_t * MAC)
 {
   for(uint8_t i= 0; i < 6; i++) mMAC[i] = MAC[i];
   Update();
 }


private:
  IPAddress mIP;
  IPAddress mNetmask;
  IPAddress mGateway;
  IPAddress mDNS;
  uint8_t mMAC[6];

  void Update()
  {
    for(uint8_t i= 0; i < 4; i++) EEPROM.update(EEPROM_IP+i,mIP[i]);
    for(uint8_t i= 0; i < 4; i++) EEPROM.update(EEPROM_IPMASK+i,mNetmask[i]);
    for(uint8_t i= 0; i < 4; i++) EEPROM.update(EEPROM_GATEWAY+i,mGateway[i]);
    for(uint8_t i= 0; i < 4; i++) EEPROM.update(EEPROM_DNS+i,mDNS[i]);
    for(uint8_t i= 0; i < 6; i++) EEPROM.update(EEPROM_MAC+i,mMAC[i]);    
  }
};

#endif // NETWORK_H
