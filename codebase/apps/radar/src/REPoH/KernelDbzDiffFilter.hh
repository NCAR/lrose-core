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
/**
 * @file KernelDbzDiffFilter.hh 
 * @brief information used to filter out dbz differences 
 * @class KernelDbzDiffFilter
 * @brief information used to filter out dbz differences 
 * 
 */

#ifndef KernelDbzDiffFilter_H
#define KernelDbzDiffFilter_H

/*----------------------------------------------------------------*/
class KernelDbzDiffFilter
{
public:

  KernelDbzDiffFilter(const bool debug);

  ~KernelDbzDiffFilter();

  void inc(const double v, const int i);

  /**
   * @return true if filtering should stop now
   */
  bool finish(const double diff_threshold);

  /**
   * @return index to remove
   */
  int choose_remove_index(void) const;

protected:
private:

  int _i_min; // index to minimum data value.
  int _i_max; // index to maximum data value.
  double _min; // min daata value
  double _max; // max data value
  double _mean; // mean data value
  double _num; // counter
  bool _first;

  bool _debug;
};

#endif
 
