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
/************************************************************************
 * MedianFilter: Object which performs a median filter on the elevation
 *               and azimuth angles in the received radar messages.
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2006
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MedianFilter_hh
#define MedianFilter_hh

#include <cstdio>
#include <list>

#include "HiqMsg.hh"


using namespace std;


class MedianFilter
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructors
   */

  MedianFilter();


  /*********************************************************************
   * Destructor
   */

  virtual ~MedianFilter();


  /*********************************************************************
   * init() - Initialize the filter parameters.
   */

  void init(const int num_before_beams = 0,
	    const int num_after_beams = 0,
	    const bool filter_elevations = false,
	    const bool filter_azimuths = false,
	    const bool debug = false)
  {
    _numBeforeBeamsParam = num_before_beams;
    _numAfterBeamsParam = num_after_beams;
    _numBeamMsgsNeeded = num_after_beams + 1;
    _filterElevations = filter_elevations;
    _filterAzimuths = filter_azimuths;

    _nextMsg = _msgList.end();
    _numBeamMsgs = 0;
    
    _debug = debug;
    
  }
  

  /*********************************************************************
   * addMsg() - Add the given message to the filter.
   */

  void addMsg(HiqMsg *current_msg);
  

  /*********************************************************************
   * getNextMsg() - Get the next message to be processed.
   *
   * Returns a pointer to the next message if there is one, 0 if we aren't
   * ready to process a message.
   */

  HiqMsg *getNextMsg();
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  list< HiqMsg* > _msgList;
  list< HiqMsg* >::iterator _nextMsg;
  
  int _numBeamMsgs;
  int _numAfterBeamMsgs;
  int _numBeamMsgsNeeded;
  int _numBeforeBeamsParam;
  int _numAfterBeamsParam;
  bool _filterElevations;
  bool _filterAzimuths;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _calcMedians() - Calculate the elevation and azimuth medians.
   */

  void _calcMedians(double &elevation_median,
		    double &azimuth_median) const;
  

};

#endif
