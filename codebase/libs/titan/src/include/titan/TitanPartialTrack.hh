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
////////////////////////////////////////////////////////////////////
// <titan/TitanPartialTrack.hh>
//
// Complex track object for TitanServer
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// Jan 2001
//
////////////////////////////////////////////////////////////////////

#ifndef TitanPartialTrack_HH
#define TitanPartialTrack_HH


#include <titan/TitanServer.hh>
#include <rapmath/bd_tree.h>
using namespace std;

class TitanPartialTrack
{

public:

  // constructor
  
  TitanPartialTrack(bool debug = false);
  
  // destructor
  
  virtual ~TitanPartialTrack();

  // clear the object
  
  void clear();

  // identify a partial track
  // Returns 0 on success, -1 on failure
  
  int identify(time_t partial_time,
	       int past_period,
	       int future_period,
	       int target_complex_num,
	       int target_simple_num,
	       const TitanServer &tserver);
  
  // Is this entry in the partial track?
  // Returns true or false

  bool entryIncluded(const track_file_entry_t &entry) const;

protected:

  bool _debug;

  int _past_period;
  int _future_period;
  int _tag;
  time_t _start_time;
  time_t _end_time;
  int _complex_num;
  int _simple_num;
  
  int _n_sparams_alloc;
  simple_track_params_t *_sparams;
  bd_tree_handle_t _tree;

 
  void _allocSparams(int n_simple_tracks);

private:
  
  // Private methods with no body - do not use

  TitanPartialTrack(const TitanPartialTrack &rhs);
  TitanPartialTrack &operator=(const TitanPartialTrack &rhs);

};

#endif
















