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
 * @file PercentileStatCalc.hh
 *
 * @class PercentileStatCalc
 *
 * Class for calculating the percentile value statistic.
 *  
 * @date 8/31/2011
 *
 */

#ifndef PercentileStatCalc_HH
#define PercentileStatCalc_HH

#include "StatCalc.hh"

using namespace std;


/** 
 * @class PercentileStatCalc
 */

class PercentileStatCalc : public StatCalc
{
 public:

  /**
   * @brief Constructor
   */

  PercentileStatCalc(const double percentile);
  
  /**
   * @brief Destructor
   */

  virtual ~PercentileStatCalc();
  
  /**
   * @brief Calculate the statistic using the given data values.
   *
   * @param[in] values    Data values to use.
   *
   * @return Returns the calculated statistic on success, BAD_DATA_VALUE
   *         on failure.
   */

  virtual double calculate(const vector< double > &values) const;

protected:

  /**
   * @brief Desired percentile.
   */

  double _percentile;
  
};


#endif
