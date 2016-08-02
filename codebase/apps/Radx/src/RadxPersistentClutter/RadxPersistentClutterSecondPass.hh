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
 * @file RadxPersistentClutterSecondPass.hh
 * @brief The second pass algorithm
 * @class RadxPersistentClutterSecondPass
 * @brief The second pass algorithm
 *
 * The algorithm uses the internal state to go back through the data and
 * create histograms at each point with clutter, which are then used to
 * estimate a clutter value, which is output.
 */

#ifndef RADXPERSISTENTCLUTTERSECONDPASS_H
#define RADXPERSISTENTCLUTTERSECONDPASS_H

#include "RadxPersistentClutter.hh"
#include "RayHistoInfo.hh"
#include <Radx/RadxVol.hh>
#include <map>

class RadxPersistentClutterSecondPass : public RadxPersistentClutter
{
  
public:

  /**
   * Constructor
   * @param[in] p  Object to use as base class
   *
   * Input contains the results of the first pass
   */
  RadxPersistentClutterSecondPass (const RadxPersistentClutter &p);

  /**
   * Destructor
   */
  virtual ~RadxPersistentClutterSecondPass(void);

  #include "RadxPersistentClutterVirtualMethods.hh"

protected:
private:

  RadxVol _templateVol;  /**< Saved volume used to form output */

  std::map<RadxAzElev, RayHistoInfo> _histo; /**< The storage of all info needed to
					  * do the computations, running counts 
					  * and histograms through time, one 
					  * object per az/elev */

};

#endif
