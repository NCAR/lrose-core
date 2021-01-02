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
 *   $Date: 2016/03/04 02:01:42 $
 *   $Id: Stat.hh,v 1.2 2016/03/04 02:01:42 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Stat: Base class for statistics calculators
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Stat_H
#define Stat_H

#include <iostream>
#include <string>

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>

using namespace std;


class Stat
{
  
public:

  //////////////////////
  // Public constants //
  //////////////////////

  static const double MISSING_DATA_VALUE;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  Stat(const string &stat_name,
       const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~Stat();


  /*********************************************************************
   * calcStatistic() - Calculate the given statistic on the given data
   *                   values.
   *
   * Returns the calculated statistic on success, MISSING_DATA_VALUE on
   * failure.
   */

  virtual double calcStatistic(const vector< double > &data_values) = 0;
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getName() - Get the name of the statistic
   *
   * Returns the statistic name
   */

  virtual string getName() const
  {
    return _name;
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _name;
  

};

#endif
