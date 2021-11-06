#include "common.h"

#include <ProcessScheduler.h>
#include "CommDefs.h"
#include <Watchdog.h>

class tWatchdogProcess : public  Process
{
  public:
  tWatchdogProcess(Scheduler &manager) : Process(manager, WATCHDOG_PRIORITY, SERVICE_SECONDLY, RUNTIME_FOREVER) {}
  virtual void setup()
  {
    watchdog.enable(Watchdog::TIMEOUT_4S);
  }
  virtual void service()
  {
    watchdog.reset();
  }
  private:
  Watchdog watchdog;
};
