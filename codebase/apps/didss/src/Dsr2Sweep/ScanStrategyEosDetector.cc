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
////////////////////////////////////////////////////////////////////////
// ScanStrategyEosDetector - Class that detects an end-of-sweep condition
//                           based on a supplied scan strategy.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2006
//
///////////////////////////////////////////////////////////////////////

#include "ScanStrategyEosDetector.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

ScanStrategyEosDetector::ScanStrategyEosDetector(const vector< double > scan_strategy,
						 const double scan_strategy_epsilon,
						 const bool debug_flag) :
  EosDetector(debug_flag),
  _scanStrategy(scan_strategy),
  _scanStrategyEpsilon(scan_strategy_epsilon),
  _prevElevNum(-1)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

ScanStrategyEosDetector::~ScanStrategyEosDetector()
{
}


/*********************************************************************
 * isNewSweep() - Returns true if this message is the beginning of the
 *                next sweep.
 */
  
bool ScanStrategyEosDetector::isNewSweep(DsRadarMsg &radar_msg)
{
  DsBeamHdr_t *beam_hdr = radar_msg.getRadarBeam().getBeamHdr();
  
  int curr_elev_num = _getElevNum(beam_hdr->elevation);
  if (beam_hdr->tilt_num < 0)
    beam_hdr->tilt_num = curr_elev_num;
  
//  if (_debug)
//    cerr << "prev elev num = " << _prevElevNum
//	 << ", curr elev num = " << curr_elev_num << endl;
  
  if (curr_elev_num < 0 || curr_elev_num == _prevElevNum)
    return false;
  
  // Check to see if we are going back to the beginning of the volume

  if (curr_elev_num > 0 && curr_elev_num < _prevElevNum)
  {
    beam_hdr->tilt_num = -1;
    return false;
  }
  
  if (_debug)
  {
    cerr << "--------------------- End of Sweep ---------------------------" << endl;
    cerr << "prev elev num = " << _prevElevNum << endl;
    cerr << "curr elev num = " << curr_elev_num << endl;
  }
  
  _prevElevNum = curr_elev_num;
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
