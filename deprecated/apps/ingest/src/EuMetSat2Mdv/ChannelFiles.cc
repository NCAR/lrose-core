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
//////////////////////////////////////////////////////////
// ChannelFiles.cc
//
// Object for combining data from channel files into a single grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#include "ChannelFiles.hh"
#include <iostream>
#include <cmath>
#include <toolsa/DateTime.hh>

using namespace std;

// Constructor

ChannelFiles::ChannelFiles(const string &prog_name,
                           const Params &params,
                           const Channel *channel,
                           int n_points_grid,
                           const double *x_lookup,
                           const double *y_lookup) :
        _progName(prog_name),
        _params(params),
        _channel(channel),
        _nPointsGrid(n_points_grid),
        _xLookup(x_lookup),
        _yLookup(y_lookup)
  
{

  _data = new ui16[_nPointsGrid];
  clear();

}

// Destructor

ChannelFiles::~ChannelFiles()

{

  if (_data != NULL) {
    delete[] _data;
  }

}

///////////////////////////////////////////////////////////
// Clear grid and other members
 
void ChannelFiles::clear()

{
  
  _dataStartTime = 0;
  _startSegmentNum = 0;
  _endSegmentNum = 0;
  _nSegmentsExpected = 0;
  _nSegmentsFound = 0;
  
  _fileList.clear();

  memset(_data, 0, _nPointsGrid * sizeof(ui16));

}
  
///////////////////////////////////////////////////////////
// Prepare for adding data
 
void ChannelFiles::_prepareForAdd(const Hrit &hrit)

{

  clear();

  _dataStartTime = hrit.getAnnotTime();
  _startSegmentNum = hrit.getPlannedStartSegmentSeqNum();
  _endSegmentNum = hrit.getPlannedEndSegmentSeqNum();
  _nSegmentsExpected = _endSegmentNum - _startSegmentNum + 1;

}
  
///////////////////////////////////////////////////////////
// add a file to the set, using the segment number as a key
//
// returns 0 on success, -1 if already in the set

int ChannelFiles::addFile(const string &path,
                          const Hrit &hrit)

{

  // check channel name, otherwise reject the file
  
  if (hrit.getAnnotChannel() != _channel->getName()) {
    if (_params.debug >= Params::DEBUG_VERBOSE_2) {
      cerr << "ChannelFiles::addFile" << endl;
      cerr << "  Rejecting file: " << path << endl;
    }
    return -1;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "ChannelFiles::addFile" << endl;
    cerr << "  Accepting file: " << path << endl;
  }

  // Check if the start time has changed.
  // If so, prepare for new data.
  
  if (_dataStartTime != hrit.getAnnotTime()) {
    _prepareForAdd(hrit);
  }

  if (_fileList.find(hrit.getSegmentSeqNum()) == _fileList.end()) {
    
    MapEntry entry(hrit.getSegmentSeqNum(), path);
    _fileList.insert(entry);
    _nSegmentsFound++;
    _handleAdd(hrit);

  } else {

    if (_params.debug) {
      cerr << "WARNING - ChannelFiles::addFile" << endl;
      cerr << "  Channel: " << _channel->getName() << endl;
      cerr << "  Data start time: " << DateTime::strn(_dataStartTime) << endl;
      cerr << "  Trying to add file more than once" << endl;
      cerr << "  Path: " << path << endl;
    }
    return -1;

  }

  return 0;

}

///////////////////////////////////////////////////////////
// Check to see whether set is complete
//
// Returns true if all files in the set are present
 
bool ChannelFiles::complete() const

{

  if (_nSegmentsFound > 0 && _nSegmentsFound == _nSegmentsExpected) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////////////////////
// Handle the add function
 
void ChannelFiles::_handleAdd(const Hrit &hrit)

{

  // load up data grid

  const ui16 *rawData = hrit.getData();

  int nCols = hrit.getNCols();
  int nLines = hrit.getNLines();
  int nPixels = nCols * nLines;

  int coff = hrit.getColOffset();
  int loff = hrit.getLineOffset();

  double cfac = hrit.getColScalingFactor();
  double lfac = hrit.getLineScalingFactor();

  double two_to_minus_sixteen = pow(2.0, -16.0);

  if (_params.debug >= Params::DEBUG_VERBOSE_2) {
    cerr << "  Adding channel: " << _channel->getName() << endl;
    cerr << "    nCols, nLines: " << nCols << ", " << nLines << endl;
    cerr << "    coff, loff: " << coff << ", " << loff << endl;
    cerr << "    cfac, lfac: " << cfac << ", " << lfac << endl;
  }

  for (int ii = 0; ii < _nPointsGrid; ii++) {

    int col = coff + (int) (_xLookup[ii] * two_to_minus_sixteen * cfac + 0.5);
    int line = loff + (int) (_yLookup[ii] * two_to_minus_sixteen * lfac + 0.5);

    if (col >= 0 && col < nCols && line >= 0 && line < nLines) {
      int rawIndex = line * nCols + col;
      if (rawIndex < nPixels) {
        _data[ii] = rawData[rawIndex];
      }
    } 

  } // ii

}
  
