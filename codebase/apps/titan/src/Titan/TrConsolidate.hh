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
// TrConsolidate.hh
//
// TrConsolidate class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef TrConsolidate_HH
#define TrConsolidate_HH

#include "Worker.hh"
#include "TrStorm.hh"

#include <vector>
using namespace std;

////////////////////////////////
// TrConsolidate

class TrConsolidate : public Worker {
  
public:

  // constructor

  TrConsolidate(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~TrConsolidate();

  // run

  // This routine searches for groups of storms which are part of a
  // common merger/split. The complex tracks in this group are
  // consolidated.
  // Returns 0 on success, -1 on failure.

  int run(TitanTrackFile &tfile,
	  vector<TrStorm*> &storms1,
	  vector<TrStorm*> &storms2,
	  vector<track_utime_t> &track_utime);

  protected:
  
private:

  vector<int> _complexNums;
  vector<int> _storm1Nums;

  // static function for qsort

  static int _compare_si32s(const void *v1, const void *v2);

  // private functions

  int _consolidate(TitanTrackFile &tfile,
		   vector<TrStorm*> &storms1,
		   int lower_track_num,
		   int higher_track_num,
		   vector<track_utime_t> &track_utime);

  void _process_storms1_entry(vector<TrStorm*> &storms1,
			      vector<TrStorm*> &storms2,
			      int storm1_num,
			      int storm2_num);
  
  void _process_storms2_entry(vector<TrStorm*> &storms1,
			      vector<TrStorm*> &storms2,
			      int storm1_num,
			      int storm2_num);

};

#endif



