/**
 * @file InterfaceAlgorithm.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/InterfaceAlgorithm.hh>
#include <FiltAlgVirtVol/AlgorithmParams.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>

//------------------------------------------------------------------
bool InterfaceAlgorithm::algInit(const std::string &appName,
				 const AlgorithmParams &p, 
				 void cleanup(int))
{
  PMU_auto_init(appName.c_str(), p.instance, 60);
  PORTsignal(SIGQUIT, cleanup);
  PORTsignal(SIGTERM, cleanup);
  PORTsignal(SIGINT, cleanup);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set up debugging state for logging
  LogMsgStreamInit::init(p.debug_mode == AlgorithmParams::DEBUG ||
			 p.debug_mode == AlgorithmParams::DEBUG_VERBOSE,
			 p.debug_mode == AlgorithmParams::DEBUG_VERBOSE,
			 true, true);
  LOG(DEBUG) << "setup";
  return true;
}

//------------------------------------------------------------------
void InterfaceAlgorithm::algFinish(void)
{
  PMU_auto_unregister();
}
