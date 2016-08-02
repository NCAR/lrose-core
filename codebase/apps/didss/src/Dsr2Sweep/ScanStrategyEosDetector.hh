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
// ScanStrategyEosDetector - Class that detects an end-of-sweep condition
//                           based on a supplied scan strategy.
//
// Nancy Rehak, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////

#ifndef ScanStrategyEosDetector_H
#define ScanStrategyEosDetector_H

#include <vector>

#include "EosDetector.hh"

using namespace std;


class ScanStrategyEosDetector : public EosDetector
{
  
public:

  /*********************************************************************
   * Constructors
   */
  
  ScanStrategyEosDetector(const vector< double > scan_strategy,
			  const double scan_strategy_epsilon,
			  const bool debug_flag = false);


  /*********************************************************************
   * Destructor
   */
  
  virtual ~ScanStrategyEosDetector();


  /*********************************************************************
   * isNewSweep() - Returns true if this message is the beginning of the
   *                next sweep.
   */
  
  virtual bool isNewSweep(DsRadarMsg &radar_msg);


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  vector< double > _scanStrategy;
  double _scanStrategyEpsilon;
  
  int _prevElevNum;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _getElevNum() - Get the index for the given elevation angle in the
   *                 scan strategy.  If the angle isn't found in the scan
   *                 strategy, return -1.
   */
  
  int _getElevNum(const double elev_angle)
  {
    for (size_t i = 0; i < _scanStrategy.size(); ++i)
      if (elev_angle >= _scanStrategy[i] - _scanStrategyEpsilon &&
	  elev_angle <= _scanStrategy[i] + _scanStrategyEpsilon)
	return i;
    
    return -1;
  }
  
};

#endif

