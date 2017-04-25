/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1999
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1999/03/14 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// MyThread.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2017
//
///////////////////////////////////////////////////////////////
//
// Class for testing TaThread
//
///////////////////////////////////////////////////////////////

#ifndef MyThread_hh
#define MyThread_hh

#include <toolsa/TaThread.hh>
class Params;

using namespace std;

class MyThread : public TaThread 
{

public:

  // constructor
  
  MyThread(const Params &params);
  
  // destructor
  
  virtual ~MyThread();

  // set start and end gates

  void setStartVal(double val) { _startVal = val; }
  void setEndVal(double val) { _endVal = val; }

  // get count and sum

  int getCount() const { return _count; }
  double getSum() const { return _sum; }

  // override run method
  
  virtual void run();
  
private:

  const Params &_params;
  double _startVal, _endVal;
  int _count;
  double _sum;
  
};

#endif

