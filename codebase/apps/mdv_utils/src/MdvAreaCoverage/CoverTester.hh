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
 *   $Date: 2016/03/04 02:22:10 $
 *   $Id: CoverTester.hh,v 1.2 2016/03/04 02:22:10 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * CoverTester: Base class for classes that create the scatter plots.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef CoverTester_H
#define CoverTester_H

#include <iostream>
#include <string>

#include <Mdv/MdvxField.hh>
#include <toolsa/Path.hh>

using namespace std;


class CoverTester
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  CoverTester();
  

  /*********************************************************************
   * Destructor
   */

  virtual ~CoverTester();


  /*********************************************************************
   * setBadValues() - Set the bad and missing data values to be used for
   *                  the inclusion testing.
   */

  void setBadValues(const double bad_value,
		    const double missing_value)
  {
    _badDataValue = bad_value;
    _missingDataValue = missing_value;
  }
  


  /*********************************************************************
   * isIncluded() - Determine whether the given data value should be included
   *                in the coverage calculation.
   *
   * Returns true if it should be included, false otherwise.
   */

  virtual bool isIncluded(const double data_value) = 0;


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  double _badDataValue;
  double _missingDataValue;
  
};

#endif
