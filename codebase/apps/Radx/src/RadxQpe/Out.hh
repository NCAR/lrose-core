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
 * @file Out.hh
 * @brief The data at one location, all elevations
 * @class Out
 * @brief The data at one location, all elevations
 */

#ifndef OUT_HH
#define OUT_HH

#include <map>
#include <string>

class VertData;
class VertPrecipData;
class VertData1;
class Sweep;
class Parms;

class Out
{
public:

  /**
   * Sets all member values
   */
  Out (const VertData &v, const Parms &parms);

  /**
   * Destructor
   */
  ~Out(void);

  /**
   * Store member values to out
   */
  void store(int igt, int iaz, const Parms &parms, Sweep &out) const;

  /**
   * For named input precip, store named output to out, using local members
   */
  void storePrecip(const std::string &name, const VertPrecipData &pdata,
		   int igt, int iaz, const Parms &parms, Sweep &out) const;

protected:
private:

  int _index;  /**< Index to height at which precip can be used */
  double _elevDeg;
  double _heightKm;
  double _rangeKm;
  double _pid;
  double _nblock;
  double _pcappi;
  double _nlow_snr;
  double _nclutter;
  double _peak;

  bool _evaluate(const VertData1 &v, const Parms &parms);
};

#endif
