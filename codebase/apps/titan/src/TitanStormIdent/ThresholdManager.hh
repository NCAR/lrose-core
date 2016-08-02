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
// ThresholdManager.hh
//
// ThresholdManager class
//
// Dave Albo
//
// August, 2014
//
///////////////////////////////////////////////////////////////

#ifndef ThresholdManager_HH
#define ThresholdManager_HH

#include "Worker.hh"

////////////////////////////////
// ThresholdManager

class ThresholdManager : public Worker {
  
public:

  // constructor
  
  ThresholdManager(const string &prog_name, const Params &params);

  // destructor
  
  virtual ~ThresholdManager();

  void set_to_defaults(void);

  void set_archive_rerun(const time_t &archive_t1);

  void set_dynamically(const time_t &gt, int lt);

  int num_intervals(void) const;

  inline double get_low_threshold(void) const {return _threshold_low;}
  inline double get_high_threshold(void) const {return _threshold_high;}
  inline double get_hist_interval(void) const {return _hist_interval;}


protected:
  
private:

  double _threshold_low;
  double _threshold_high;
  double _hist_interval;
  bool _is_archive_rerun;
  time_t _archive_t1;

  void _set_hist_intervals(void);
};

#endif



