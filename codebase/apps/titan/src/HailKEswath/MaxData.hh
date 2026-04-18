/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 13:58:59
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// MaxData.hh
//
// Max data object
//
/////////////////////////////////////////////////////////////

#ifndef MaxData_H
#define MaxData_H

#include "Params.hh"
#include <Mdv/DsMdvx.hh>

#include <string>
using namespace std;

class MaxData {
  
public:
  
  MaxData(const string &prog_name, const Params &params);
  
  ~MaxData();
  
  void setTargetPeriod(double period);

  int processFile(const string &file_path);
  
  int dataFound() { return (_dataFound); }
  
  const Mdvx::coord_t &grid() { return (_grid); }
  
  time_t latestDataTime() { return (_latestDataTime); }

  time_t dataStartTime() { return _dataStartTime; }
  time_t dataEndTime() { return _dataEndTime; }
  time_t dataCentroidTime() { return _dataCentroidTime; }

  fl32 *maxHailKeFlux() { return _maxHailKeFlux; }
  fl32 *maxHailMassFlux() { return _maxHailMassFlux; }

protected:
  
private:

  const string &_progName;
  const Params &_params;
  
  bool _dataFound;   // flag to indicate the some data has been found
  
  Mdvx::coord_t _grid; // coord grid from first file read

  int _nxy;

  fl32 *_maxHailKeFlux;     // max HailKeFlux data over period
  fl32 *_maxHailMassFlux;   // array for MaxDbz over period

  time_t _latestDataTime;
  time_t _dataStartTime;
  time_t _dataEndTime;
  time_t _dataCentroidTime;

  double _targetAccumPeriod;
  double _actualAccumPeriod;
  double _volDuration;

  void _updateMaxHailKeFlux(const MdvxField &);
  void _updateMaxHailMassFlux(const MdvxField &);
  
};

#endif
