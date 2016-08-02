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

#ifndef pairMgr_H
#define pairMgr_H

#include <string>
#include <vector>

#include <Spdb/DsSpdb.hh>
#include "Params.hh"

#include "pairAverage.hh" 

using namespace std;

class pairMgr {

public:

  // Constructor. - Gets ready.
  //
  pairMgr( Params *params,
	      int param_index);
	   
  //
  // Destructor
  //
  ~pairMgr();
  
  // - Gets data.
  void load(time_t startTime, time_t endTime);
  
  // Get the number of averaging bins.
  int getNumBins();

  // Get the nth Bin Average.
  pairAverage *getAverage(int n);
  
  // Get the Whole container of Averaging Bins
  vector <pairAverage *> getBins();
  
  //  set the averaging bins to initial state.
  void clear_bins();

  void set_times(time_t start, time_t end);

  // Return the label for the point we used.
  string getLabel();
   
  Params *_params;
  
  string _url;
  string _label;
  string _color;
  int _data_type;
  int _line_style;

  int _interval; // Binning interval seconds
  int _time_allowance; // Allowed offset between obs time and model time
  time_t _start_time;
  time_t _end_time;
  vector <pairAverage *> _ave; // Contains averages for each bin.

  private :
 };

#endif

