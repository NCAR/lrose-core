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
// ChannelFiles.hh
//
// Object for combining data from channel files into a single grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2005
//
//////////////////////////////////////////////////////////

#ifndef ChannelFiles_HH
#define ChannelFiles_HH

#include <string>
#include <map>
#include "Params.hh"
#include "Channel.hh"
#include "Hrit.hh"

using namespace std;

class ChannelFiles {
  
public:

  // constructor
  
  ChannelFiles(const string &prog_name,
               const Params &params,
               const Channel *channel,
               int n_points_grid,
               const double *x_lookup,
               const double *y_lookup);

  // Destructor
  
  virtual ~ChannelFiles();
  
  // Clear data
  
  void clear();

  // add a file
  // returns 0 on success, -1 if already in the set
  
  int addFile(const string &path,
              const Hrit &hrit);

  // Check to see whether set is complete
  // Returns true if all files in the set are present
  
  bool complete() const;

  // get at the data

  const ui16 *getData() const { return _data; }

  // get the channel

  const Channel *getChannel() const { return _channel; }

  // get the channel name

  const string &getChannelName() const { return _channel->getName(); }

private:
  
  typedef map<int, string, less<int> > FileMap;
  typedef pair<int, string > MapEntry;
  
  const string &_progName;
  const Params &_params;

  const Channel *_channel;
  
  int _nPointsGrid;
  ui16 *_data;
  const double *_xLookup;
  const double *_yLookup;
  
  FileMap _fileList;

  time_t _dataStartTime; // time of start of data in sat files
  
  int _startSegmentNum;
  int _endSegmentNum;
  int _nSegmentsExpected;
  int _nSegmentsFound;

  void _prepareForAdd(const Hrit &hrit);
  void _handleAdd(const Hrit &hrit);

};

#endif



