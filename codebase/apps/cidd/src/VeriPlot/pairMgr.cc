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
//
// Object that creates and stores the sounding objects.
//

#include "pairMgr.hh"
#include "pairAverage.hh"
#include <X11/Xlib.h>

using namespace std;


//
// Constructor.  Determines which data to read.
// Reads the data and fills up the vector.
//
pairMgr::pairMgr(Params *params,
		int param_index) // Which URL in param file this Mgr instance should use.
{ 
  _params = params;
  _interval =  _params->bin_interval;
  _time_allowance = _params->valid_range;

  _label = _params->_inputDataSrc[param_index].label;
  _url = _params->_inputDataSrc[param_index].url;
  _data_type = _params->_inputDataSrc[param_index].data_type;
  _color = _params->_inputDataSrc[param_index].color;
  _line_style = _params->_inputDataSrc[param_index].line_style;
}

//
// LOAD 
//     Instantiate Pair Averaging Bin containers 
//     Load them from the Genpt 
void pairMgr::load(time_t startTime, time_t endTime)
{
  GenPt  pt;
  DsSpdb input_spdb;

  input_spdb.getInterval(_url, startTime, endTime, _data_type, 0, false);

  const vector<Spdb::chunk_t> &chunks = input_spdb.getChunks();
  clear_bins();  // Clear out all old data;

  if(_params->debug ) {
    cerr << _url << " - Found ";
    cerr << input_spdb.getNChunks() << " Records ";
    cerr << endl;
   }

  for (int ichunk = 0; ichunk < input_spdb.getNChunks(); ichunk++){
       pairAverage *pA = new pairAverage(_params,chunks[ichunk].valid_time,chunks[ichunk].expire_time);
       // Dereference the binary record
       if(chunks[ichunk].data != NULL && pt.disassemble(chunks[ichunk].data,chunks[ichunk].len) == 0 ) {
	      pA->load(pt);
	      // Add the Completed Bin to our Container of Bins
              _ave.push_back(pA);
	}
  }
}
  
//
// Destructor
//
pairMgr::~pairMgr(){
  for (unsigned is=0; is < _ave.size(); is++){
    delete _ave[is];
  }
}

// set the averaging bins to initial state.
void pairMgr::clear_bins(){
  for (unsigned is=0; is < _ave.size(); is++){
   delete _ave[is];
   }
}

//  Set the start and end times for the pair averaging.
void pairMgr::set_times(time_t start, time_t end) {
  _start_time = start;
  _end_time = end;
}

// Get the number bins.
//
int pairMgr::getNumBins(){
  return _ave.size();
}
//
// Get the nth sounding.
pairAverage *pairMgr::getAverage(int n){
  return _ave[n];
}

// Get the Vector of Averaging bins.
vector <pairAverage *> pairMgr::getBins(){
  return _ave;
}

// Get the label in use.
string pairMgr::getLabel(){
  return _label;
}
