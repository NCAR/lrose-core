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
 *
 * @file GTFilter.hh
 *
 * @class GTFilter
 *
 * GTFilter handles filtering of radar data, using a greater than comparison.
 *  
 * @date 9/18/2002
 *
 */

#ifndef GTFilter_HH
#define GTFilter_HH

using namespace std;

#include "Filter.hh"


/** 
 * @class GTFilter
 */

class GTFilter : public Filter
{
  public:

  ////////////////////
  // Public methods //
  ////////////////////

  /** 
   * @brief Constructor
   */

  GTFilter();

  /**
   * @brief Destructor
   */

  virtual ~GTFilter();


  /**
   * @brief Get the filter flag for this gate.
   *
   * @param[in] beam_num Beam number of gate in the interest data.
   * @param[in] gate_num Gate number of gate.
   *
   * @return Returns true if the filter indicates that this gate should be
   *         filtered, false otherwise.
   */

  virtual bool getFilterFlag(const int beam_num, const int gate_num);

};

#endif

