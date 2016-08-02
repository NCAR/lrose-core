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
 *   $Id: MinComputer.hh,v 1.2 2016/03/04 02:22:10 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * MinComputer: Class that computes the 2D value as the minimum data value
 *              in each column.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef MinComputer_H
#define MinComputer_H

#include "Computer.hh"

using namespace std;


class MinComputer : public Computer
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  MinComputer();
  

  /*********************************************************************
   * Destructor
   */

  virtual ~MinComputer();


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  fl32 _currentMin;
  bool _valueFound;
  

  ///////////////////////////////
  // Protected virtual methods //
  ///////////////////////////////

  /*********************************************************************
   * _initCompute() - Initialize the computation for this data column.
   */

  virtual void _initCompute();
  

  /*********************************************************************
   * _addValue() - Add the given value to the current column computation.
   */

  virtual void _addValue(const fl32 data_value);
  

  /*********************************************************************
   * _computeValue() - Compute the final 2D value.
   *
   * Returns the value computed.
   */

  virtual fl32 _computeValue();
  

};

#endif
