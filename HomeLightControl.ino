#include "common.h"

#include <ProcessScheduler.h>
#include "CommReciever.h"
#include "CommSender.h"
#include <Arduino.h>
#include "Eeprom.h"
#include "WorkerProcess.h"
#include "DigitalInputProcess.h"
#include "OutputProcess.h"
#include "WatchdogProcess.h"

#ifdef CONTROLLER
#include "network.h"
#include "TelnetServer.h"
#endif


 
Scheduler sched;
CommRecieverProcess CommReciever(sched);
CommSenderProcess CommSender(sched);
WorkerProcess Worker(sched);

#ifdef CONTROLLER
tNetwork Network;
tTcpServerProcess TcpServerProcess(sched);
tTelnetServer TelnetServer;

#else
DigitalInputProcess DigitalInput(sched);
#endif

tOutputProcess OutputProcess(sched);
tWatchdogProcess  WatchdogProcess(sched);

void COMM_SERIAL_EVENT() {
  CommReciever.serialEvent();
}





void setup() {
  if (EEPROM.read(EEPROM_CANNARY_OFFSET) != EEPROM_CANNARY)
    SetDefaultEEPromValues();

  COMM_SERIAL.begin(9600);
  while (!COMM_SERIAL);
#ifdef DEBUG_SERIAL
  DEBUG_SERIAL.begin(115200);
  while (!DEBUG_SERIAL);
#endif

  CommSender.add();
  CommReciever.add();
  Worker.add();
  WatchdogProcess.add(true);

#ifdef CONTROLLER
  Network.init();
  TcpServerProcess.add(true);
#else  
  DigitalInput.add(true);
#endif

  OutputProcess.add(true);
}

void loop() {
  sched.run();
}
