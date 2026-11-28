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
 *   $Date: 2016/03/04 02:22:11 $
 *   $Id: Input.hh,v 1.2 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Input: Base class for input dataset objects.
 *
 * RAP, NCAR, Boulder CO
 *
 * November 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Input_HH
#define Input_HH

#include <vector>

#include <toolsa/DateTime.hh>

#include "DataPoint.hh"

using namespace std;


class Input
{
 public:

  /////////////////////////////
  // Constructors/destructor //
  /////////////////////////////

  /**********************************************************************
   * Constructor
   */

  Input(const bool debug_flag = false);
  

  /**********************************************************************
   * Destructor
   */

  virtual ~Input(void);
  

  /////////////////////
  // Utility methods //
  /////////////////////

  /**********************************************************************
   * getData() - Retrieve the input data for the given time range.
   */

  virtual vector< DataPoint > getData(const DateTime start_time,
				      const DateTime end_time) = 0;
  

  /**********************************************************************
   * getBadDataValue() - Retrieve the bad data value for this data.
   */

  virtual double getBadDataValue() = 0;
  

  /**********************************************************************
   * getMissingDataValue() - Retrieve the missing data value for this data.
   */

  virtual double getMissingDataValue() = 0;
  

  /**********************************************************************
   * getFieldNameLong() - Retrieve the long version of the field name.
   */

  virtual string getFieldNameLong() = 0;
  

  /**********************************************************************
   * getFieldName() - Retrieve the name of this field.
   */

  virtual string getFieldName() = 0;
  

  /**********************************************************************
   * getFieldUnits() - Retrieve the units for this field.
   */

  virtual string getFieldUnits() = 0;
  

  /**********************************************************************
   * getDataSource() - Retrieve the data source for this field.
   */

  virtual string getDataSource() = 0;
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;


};


#endif
