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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/06 23:53:40 $
//   $Id: MedianFilter.cc,v 1.2 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MedianFilter: Object which performs a median filter on the elevation
 *               and azimuth angles in the received radar messages.
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2006
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>
#include <iostream>
#include <vector>

#include "HiqBeamMsg.hh"
#include "MedianFilter.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

MedianFilter::MedianFilter() :
  _debug(false),
  _nextMsg(_msgList.end()),
  _numBeamMsgs(0),
  _numAfterBeamMsgs(0),
  _numBeamMsgsNeeded(1),
  _numBeforeBeamsParam(0),
  _numAfterBeamsParam(0),
  _filterElevations(false),
  _filterAzimuths(false)
{
}


/*********************************************************************
 * Destructor
 */

MedianFilter::~MedianFilter() 
{
  list< HiqMsg* >::iterator msg_ptr;
  for (msg_ptr = _msgList.begin(); msg_ptr != _msgList.end(); ++msg_ptr)
    delete *msg_ptr;
}


/*********************************************************************
 * addMsg() - Add the given message to the filter.
 */

void MedianFilter::addMsg(HiqMsg *current_msg)
{
  // Add the message to the current queue

  _msgList.push_back(current_msg);
  
  if (current_msg->getMsgType() == HiqMsg::BEAM_MSG)
  {
    ++_numBeamMsgs;
    ++_numAfterBeamMsgs;
  }
  
  if (_nextMsg == _msgList.end())
    _nextMsg = _msgList.begin();
}


/*********************************************************************
 * getNextMsg() - Get the next message to be processed.
 *
 * Returns a pointer to the next message if there is one, 0 if we aren't
 * ready to process a message.
 */

HiqMsg *MedianFilter::getNextMsg()
{
  // If we aren't filtering anything, then return the first message in
  // the queue.  It should be the only message there.

  if (!_filterElevations && !_filterAzimuths)
  {
    if (_nextMsg == _msgList.begin())
      _nextMsg = _msgList.end();
    
    if (_msgList.size() == 0)
      return 0;
    
    HiqMsg *current_msg = _msgList.front();
    _msgList.pop_front();

    return current_msg;
  }
  
  // If there aren't any messages, return nothing

  if (_nextMsg == _msgList.end())
    return 0;
  
  // If the next message in the list isn't a beam message, it can just be 
  // returned and erased from the list.

  if ((*_nextMsg)->getMsgType() != HiqMsg::BEAM_MSG)
  {
    HiqMsg *return_msg = *_nextMsg;
    list< HiqMsg* >::iterator erase_msg = _nextMsg;
    ++_nextMsg;
    _msgList.erase(erase_msg);
    
    return return_msg;
  }
  
  // If we don't have enough messages to do the filtering, return nothing

  if (_numAfterBeamMsgs < _numAfterBeamsParam)
    return 0;
  
  // Calculate the median angles

  double elevation_median, azimuth_median;
  
  _calcMedians(elevation_median, azimuth_median);
  
  // Make a copy of the next message so we can return it.  If we get here,
  // we know we have a beam message.  We need to make a copy of it to return
  // because we may need it later for filtering later angles.

  HiqBeamMsg *next_msg = (HiqBeamMsg *)(*_nextMsg);
  HiqBeamMsg *return_msg = new HiqBeamMsg(*next_msg);
  ++_nextMsg;
  --_numAfterBeamMsgs;
  
  if (_filterElevations)
    return_msg->setElevation(elevation_median);
  if (_filterAzimuths)
    return_msg->setAzimuth(azimuth_median);
  
  // Remove the first message from the queue, if appropriate

  if (_numBeamMsgs > _numBeforeBeamsParam + _numAfterBeamsParam)
  {
    if (_nextMsg == _msgList.begin())
      ++_nextMsg;
    
    delete _msgList.front();
    _msgList.pop_front();

    --_numBeamMsgs;
  }
    
  return return_msg;
}


/*********************************************************************
 * _calcMedians() - Calculate the elevation and azimuth medians.
 */

void MedianFilter::_calcMedians(double &elevation_median,
				double &azimuth_median) const
{
  vector< double > elevation_list;
  vector< double > azimuth_list;
  
  list< HiqMsg* >::const_iterator msg_iter;
  
  if (_debug)
    cerr << "---> Elevation list: ";
  
  for (msg_iter = _msgList.begin(); msg_iter != _msgList.end(); ++msg_iter)
  {
    if ((*msg_iter)->getMsgType() != HiqMsg::BEAM_MSG)
      continue;
    
    HiqBeamMsg *beam_msg = (HiqBeamMsg *)(*msg_iter);
    
    elevation_list.push_back(beam_msg->getElevation());
    azimuth_list.push_back(beam_msg->getAzimuth());

    if (_debug)
      cerr << beam_msg->getElevation() << " ";
    
  } /* endfor - msg_iter */

  if (_debug)
    cerr << endl;
  
  sort(elevation_list.begin(), elevation_list.end());
  sort(azimuth_list.begin(), azimuth_list.end());
  
  int num_angles = elevation_list.size();
  
  if (num_angles % 2 == 0)
  {
    int index1 = num_angles / 2;
    int index2 = index1 - 1;
    
    elevation_median =
      (elevation_list[index1] + elevation_list[index2]) / 2.0;
    azimuth_median =
      (azimuth_list[index1] + azimuth_list[index2]) / 2.0;
  }
  else
  {
    int index = num_angles / 2;
    
    elevation_median = elevation_list[index];
    azimuth_median = azimuth_list[index];
  }
  
  if (_debug)
    cerr << "    Elevation median: " << elevation_median << endl;
  
}
