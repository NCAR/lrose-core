/*

  rapicThread.h class
  
  A manager class to launch the main rapic class instances in
  with a thread calling the comms, db and scan mng check methods
*/

#ifndef __RAPIC_THREAD_H
#define __RAPIC_THREAD_H 

#include "freelist.h"
#include "rdrscan.h"
#include "rpcomms.h"
#include "rpdb.h"

extern RPCommMng *CommMngr;
extern scan_mng *ScanMng;

class rapicThread : public ThreadObj
{
  time_t NextCommPollTime;
  time_t timenow;
  time_t CommPollPeriod;

 public:
  rapicThread(float lp_dly = 1.0,
	      char *comminifile = NULL,
	      char *dbinifile = NULL);
  ~rapicThread();
  virtual void workProc();
}; 

#endif
