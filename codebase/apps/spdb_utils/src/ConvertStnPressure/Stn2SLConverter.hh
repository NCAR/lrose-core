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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:39:55 $
 *   $Id: Stn2SLConverter.hh,v 1.2 2016/03/07 01:39:55 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Stn2SLConverter: Class for converting the pressure values in a station
 *                  report from station pressure to sea level pressure.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Stn2SLConverter_HH
#define Stn2SLConverter_HH

#include "Converter.hh"

using namespace std;

class Stn2SLConverter : public Converter
{
 public:

  /*********************************************************************
   * Constructors
   */

  Stn2SLConverter(const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~Stn2SLConverter(void);
  

  /*********************************************************************
   * updatePressure() - Update the pressure field in the given station
   *                    report.
   */

  virtual void updatePressure(station_report_t &stn_report);
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  static const double GAMMA;
  static const double GRAVITY;
  static const double R;
  
  
};


#endif
