// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/////////////////////////////////////////////////////////////
// AccumData.hh
//
// Accumulation data object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

#ifndef AccumData_H
#define AccumData_H

#include "Params.hh"
#include <Mdv/DsMdvx.hh>

#include <string>
using namespace std;

class AccumData {
  
public:
  
  AccumData(const string &prog_name, const Params &params);
  
  ~AccumData();
  
  // init for computations
  
  void init();
  void setTargetPeriod(double period);

  // process data from a file

  int processFile(const string &file_path);
  int processFile(const string &file_path, time_t inputTime, int leadTime);
  
  // compute and write
  // returns 0 on success, -1 on failure

  int computeAndWrite(time_t start_time,
                      time_t end_time,
                      time_t centroid_time,
                      int forecast_lead_time = 0);

  // get methods

  int dataFound() { return (_dataFound); }
  
  const Mdvx::coord_t &grid() { return (_grid); }
  
  time_t latestDataTime() { return (_latestDataTime); }

  time_t dataStartTime() { return _dataStartTime; }
  time_t dataEndTime() { return _dataEndTime; }
  time_t dataCentroidTime() { return _dataCentroidTime; }
  
  fl32 *precip() { return _precip; }
  fl32 *adjusted() { return _adjusted; }
  fl32 *rate() { return _rate; }
  fl32 *maxVil() { return _maxVil; }
  fl32 *maxDbz() { return _maxDbz; }

protected:
  
private:

  const string &_progName;
  const Params &_params;
  
  bool _dataFound;   // flag to indicate the some data has been found
  
  Mdvx::coord_t _grid; // coord grid from first file read

  int _nxy;

  fl32 *_precip;   // accumulated precip data
  fl32 *_rate;     // mean precip rate
  fl32 *_vil;      // vil data for given time
  fl32 *_maxVil;   // max vil data over period
  fl32 *_adjusted; // accumulated precip data adjusted to target period
  fl32 *_maxDbz;     // array for MaxDbz over period

  time_t _latestDataTime;
  time_t _dataStartTime;
  time_t _dataEndTime;
  time_t _dataCentroidTime;

  double _targetAccumPeriod;
  double _actualAccumPeriod;
  double _volDuration;
  time_t _prevDataTime;

  void _free();

  void _updateAccumFromDbz(const MdvxField &inFld, const fl32 *dbz,
			   time_t radar_time);
  void _updateAccumFromPrecip(const MdvxField &inFld);
  void _updateAccumFromRate(const MdvxField &inFld);
  void _updateMaxDbz(const MdvxField &compFld);
  void _updateMaxVil();
  void _computeVil(const MdvxField &dbzFld);
  void _computeDbzFromVert(const MdvxField &inFld, fl32 *dbz);
  
  void _getZrParams(time_t radar_time, double &zr_coeff, double &zr_expon);

  void _computeAdjusted();
  void _computeRate();
  void _setMaxDepth(double maxDepth);
  void _normalizeByNSeasons(double nSeasons);

};

#endif
