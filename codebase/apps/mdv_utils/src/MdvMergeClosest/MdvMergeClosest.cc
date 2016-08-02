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
#include <toolsa/copyright.h>

/**
 * @file MdvMergeClosest.cc
 */

#include "MdvMergeClosest.hh"
#include <toolsa/LogMsg.hh>

//----------------------------------------------------------------
MdvMergeClosest::MdvMergeClosest(const Parms &parms) :
  _parms(parms), _first(true),   _output(parms)
{
  for (size_t i=0; i<parms._input.size(); ++i)
  {
    _data.push_back(InputData(parms._input[i], parms));
  }
}

//----------------------------------------------------------------
MdvMergeClosest::~MdvMergeClosest()
{
}

//----------------------------------------------------------------
void MdvMergeClosest::update(const time_t &t)
{
  // see if anything has changed and update the output grid accordingly.
  bool changed = false;

  for (size_t i=0; i<_data.size(); ++i)
  {
    InputData::Status_t s;
    s = _data[i].update(t);
    switch (s)
    {
    case InputData::FIRST_DATA:
      // Check projection, then add this input to the state
      if (_add(_data[i], i))
      {
	// update state for this input (change output values where appropriate)
	// (check if projection is consistent)
	_update(_data[i], i);
	changed = true;
      }
      break;
    case InputData::NEW_DATA:
      // update state for this input (change output values where appropriate)
      // (Note that changes in projection are prevented in the InputData class)
      _update(_data[i], i);
      changed = true;
      break;
    case InputData::TIMEOUT:
      // remove this data from activity in the state
      _timeout(_data[i], i);
      changed = true;
      break;
    case InputData::UNCHANGED:
    case InputData::BAD:
    default:
      // do nothing
      break;
    }
  }
  if (changed)
  {
    _output.write(t);
  }
}

//----------------------------------------------------------------
bool MdvMergeClosest::_add(Data &d, int index)
{
  LOG(LogMsg::DEBUG, "Adding first data");
  d.print();

  if (_first)
  {
    _first = false;
    _proj = d.getProjection();

    // with a projection, and lat/lon for each data, we can setup the lookups
    _lookup.init(_parms, _proj);

    // we can use this example input to initialize the output
    _output.init(d);
  }
  else
  {
    if (_proj != d.getProjection())
    {
      LOG(LogMsg::ERROR, "Unequal projection, this input not added");
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------
void MdvMergeClosest::_update(Data &d, int index)
{
  LOG(LogMsg::DEBUG, "Updating with new data");
  d.print();
  _lookup.update(index);

  // at each point if index is our best choice, update that point
  // otherwise do not update anything
  for (int y=0; y<_lookup.ny(); ++y)
  {
    for (int x=0; x<_lookup.nx(); ++x)
    {
      int ind = _lookup.bestInput(x, y);
      if (ind == index)
      {
	// update output to reflect that this input is closest to x,y
	_output.setValues(d, x, y);
      }
    }
  }
}

//----------------------------------------------------------------
void MdvMergeClosest::_timeout(Data &d, int index)
{
  LOG(LogMsg::DEBUG, "Timeout");
  d.print();
  _lookup.timeout(index);

  // not that index has been set inactive, redo every point,
  // which is easier than figuring out which ones actually change.
  for (int y=0; y<_lookup.ny(); ++y)
  {
    for (int x=0; x<_lookup.nx(); ++x)
    {
      int ind = _lookup.bestInput(x, y);
      _output.setValues(_data[ind], x, y);
    }
  }
}
