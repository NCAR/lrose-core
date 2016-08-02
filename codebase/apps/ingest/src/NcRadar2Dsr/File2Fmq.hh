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
// File2Fmq.hh
//
// File2Fmq object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2001
//
///////////////////////////////////////////////////////////////

#ifndef File2Fmq_H
#define File2Fmq_H

#include <string>
#include <vector>
#include "Params.hh"
#include "Field.hh"
#include <Fmq/DsRadarQueue.hh>
#include <netcdf.hh>
using namespace std;

////////////////////////
// This class

class File2Fmq {
  
public:

  // constructor

  File2Fmq(const Params &params,
	   const NcFile &ncf,
	   DsRadarQueue &r_queue);
  
  // destructor
  
  ~File2Fmq();

  // put the params

  int writeParams();

  // write the beam data

  int writeBeams(int vol_num, int tilt_num,
		 time_t start_time);

protected:
  
private:

  const Params &_params;
  const NcFile &_ncf;
  DsRadarQueue &_rQueue;

  int _timeDimId;
  int _maxCellsDimId;

  int _nTimes;
  int _maxCells;
  time_t _baseTime;
  double _fixedAngle;

  NcVar *_baseTimeVar;
  NcVar *_timeOffsetVar;
  NcVar *_azimuthVar;
  NcVar *_elevationVar;
  
  NcValues *_timeOffsetVals;
  NcValues *_elevationVals;
  NcValues *_azimuthVals;

  double *_timeOffsetData;
  float *_elevationData;
  float *_azimuthData;

  vector<Field *> _fields;

  void _setVars();
  void _findFields();
  int _findNameInList(const string &name,
		      const string &list);
  
  
};

#endif

