/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// TodPeriod.hh
//
// Keeps track of which Time-Of-Day period we are in.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#ifndef TodPeriod_HH
#define TodPeriod_HH

#include "HailKEswath.hh"
#include <toolsa/umisc.h>
#include <rapformats/zrpf.h>
#include <string>
using namespace std;

typedef struct {
  time_t start;
  time_t end;
} period_times_t;

////////////////////////////////
// TodPeriod - abstract base class

class TodPeriod {
  
public:

  // constructor

  TodPeriod(const string &prog_name, const Params &params);
  
  // destructor
  
  virtual ~TodPeriod();

  // Set the period times given the trigger time.
  // Returns TRUE if period number has changed, FALSE otherwise.

  int set(time_t trigger_time);

  // start and end times of current period

  time_t periodStart;
  time_t periodEnd;

  // flag for success in construction

  int OK;

protected:
  
private:

  const string &_progName;
  const Params &_params;
  int _nPeriods;
  int _periodNum;;
  period_times_t *_periods;
  zrpf_handle_t _zrHandle;

};

#endif
