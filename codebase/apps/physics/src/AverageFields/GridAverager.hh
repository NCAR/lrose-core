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
 *   $Date: 2016/03/06 23:15:37 $
 *   $Id: GridAverager.hh,v 1.3 2016/03/06 23:15:37 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GridAverager: Class for calculating averages of grids.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GridAverager_HH
#define GridAverager_HH

#include "GridCalculator.hh"

using namespace std;


class GridAverager : public GridCalculator
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  GridAverager(const bool include_missing_in_avg = false,
	       const double missing_avg_value = 0.0,
	       const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~GridAverager(void);
  

  /*********************************************************************
   * doCalculation() - Do the calculation on the given grids.  Put the
   *                   results in the given output grid.
   *
   * Note that this method assumes that all of the given grids have the
   * same projection, so the calling method must ensure that this is
   * true.
   *
   * Returns true on success, false on failure.
   */

  virtual bool doCalculation(vector< MdvxField* > input_fields,
			     MdvxField *avg_field);
  

 protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  // Algorithm parameters

  bool _includeMissingInAvg;
  double _missingAvgValue;
  

};


#endif
