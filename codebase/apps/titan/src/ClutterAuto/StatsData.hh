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
// StatsData.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2003
//
/////////////////////////////////////////////////////////////

#ifndef StatsData_HH
#define StatsData_HH

#include "Params.hh"
#include <Mdv/DsMdvx.hh>

#include <string>
#include <cmath>

using namespace std;

class StatsData {
  
public:
  
  StatsData(const string &prog_name, const Params &params);
  ~StatsData();

  // process data for a given time

  int processTime(time_t fileTime);

  // compute the stats
  // returns 0 on success, -1 on failure
  
  int compute();
  
  // get methods
  
  const Mdvx::master_header_t &getMhdr() { return _mhdr; }
  const Mdvx::field_header_t &getFhdr() { return _fhdr; }
  const Mdvx::vlevel_header_t &getVhdr() { return _vhdr; }
  
  time_t getLatestDataTime() { return (_latestDataTime); }
  time_t getDataStartTime() { return _dataStartTime; }
  time_t getDataEndTime() { return _dataEndTime; }
  time_t getDataCentroidTime() { return _dataCentroidTime; }
  
  fl32 *getFrac() { return _frac; }
  fl32 *getMean() { return _mean; }
  fl32 *getSdev() { return _sdev; }
  fl32 *getCorr() { return _corr; }

protected:
  
private:
  
  const string &_progName;
  const Params &_params;
  
  int _ntimes;         // number of good times read
  
  Mdvx::coord_t _grid; // coord grid from first file
  Mdvx::master_header_t _mhdr; // master header from first file
  Mdvx::field_header_t _fhdr; // field header from first file
  Mdvx::vlevel_header_t _vhdr; // vlevel header from first file
  
  int _npoints; // number of points in grid
  
  fl32 *_dbzThresh;    // grid holding dbz threshold values
  fl32 *_prev;         // data for prev volume
  fl32 *_frac;         // fraction used
  fl32 *_mean;         // mean
  fl32 *_sdev;         // standard deviation
  fl32 *_corr;         // auto-correlation
  
  double *_nn;
  double *_sumx;
  double *_sumy;
  double *_sumxx;
  double *_sumyy;
  double *_sumxy;

  time_t _latestDataTime;
  time_t _dataStartTime;
  time_t _dataEndTime;
  time_t _dataCentroidTime;
  
  void _initDbzThresh();

};

#endif
