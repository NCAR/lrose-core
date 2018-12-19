/**
 * @file VolumeTrigger.hh
 * @brief Triggering part of Volume
 * @class VolumeTrigger
 * @brief Triggering part of Volume
 */

#ifndef VOLUME_TRIGGER_HH
#define VOLUME_TRIGGER_HH

#include <ctime>

class VirtVolParms;
class DsUrlTrigger;


//------------------------------------------------------------------
class VolumeTrigger
{
public:

  /**
   * Constructor
   */
  VolumeTrigger(const VirtVolParms &parms, int argc, char **argv);

  /**
   * Destructor
   */
  virtual ~VolumeTrigger(void);

  /**
   * Triggering method. Waits till new data triggers a return.
   *
   * @param[out] t   time that was triggered.
   * @return true if a time was triggered, false for no more triggering.
   */
  bool trigger(time_t &t);

protected:
private:

  bool _isArchiveMode;    /**< True if it is archive mode, false for realtime*/
  time_t _archiveT0;      /**< Earliest time in archive mode */
  time_t _archiveT1;      /**< Latest time in archive mode */
  DsUrlTrigger *_T;       /**< Triggering object pointer */

};

#endif
