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
 *   $Date: 2016/03/07 01:23:06 $
 *   $Id: DataProcessor.hh,v 1.4 2016/03/07 01:23:06 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DataProcessor.hh : Base class for objects that process the data in
 *                    the Terascan files.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DataProcessor_HH
#define DataProcessor_HH


#include <string>

// Include the needed Terascan include files.  Note that they were
// written assuming that people would only write in C, not in C++.

#ifdef __cplusplus
extern "C" {
#endif

#include <gp.h>

#ifdef __cplusplus
}
#endif


#include <dataport/port_types.h>

using namespace std;


class DataProcessor
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  DataProcessor();
  

  /*********************************************************************
   * Destructor
   */

  virtual ~DataProcessor(void);
  

  /*********************************************************************
   * convertData() - Convert the indicated satellite data into float
   *                 data using the given array.
   */

  virtual bool convertData(VARP sat_var,
			   const int first_line,
			   const int first_sample,
			   const int num_lines,
			   const int num_samples,
			   float *output_data) = 0;
  

};


#endif
