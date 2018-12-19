/**
 * @file VolumeTrigger.cc
 */

//------------------------------------------------------------------
#include "VolumeTrigger.hh"
#include <FiltAlgVirtVol/VirtVolParms.hh>
#include <dsdata/DsUrlTrigger.hh>
#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>

//------------------------------------------------------------------
VolumeTrigger::VolumeTrigger(const VirtVolParms &parms, int argc, char **argv):
  _isArchiveMode(false), _T(NULL)
{
  bool error;
  DsUrlTrigger::checkArgs(argc, argv, _archiveT0, _archiveT1, _isArchiveMode,
			  error);
  if (parms.debug_triggering)
  {
    LogMsgStreamInit::setThreading(true);
  }
  if (error)
  {
    LOG(ERROR) << "ERROR parsing args";
    exit(1);
  }
  LOG(DEBUG_VERBOSE) << "------before trigger-----";

  if (_isArchiveMode)
  {
    _T = new DsUrlTrigger(_archiveT0, _archiveT1, parms.trigger_url,
			  DsUrlTrigger::OBS, parms.debug_triggering);
  }
  else
  {
    _T = new DsUrlTrigger(parms.trigger_url, DsUrlTrigger::OBS,
			  parms.debug_triggering);
  }

}

//------------------------------------------------------------------
VolumeTrigger::~VolumeTrigger(void)
{
  if (_T != NULL)
  {
    delete _T;
    _T = NULL;
  }
}

//------------------------------------------------------------------
bool VolumeTrigger::trigger(time_t &t)
{
  bool stat = _T->nextTime(t);
  if (stat)
  {
    LOG(DEBUG) << "-------Triggered " << DateTime::strn(t) << " ----------";
    return true;
  }
  else
  {
    LOG(DEBUG) << "no more triggering";
    delete _T;
    _T = NULL;
    return false;
  }
}

