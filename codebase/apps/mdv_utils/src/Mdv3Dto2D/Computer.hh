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
 *   $Id: Computer.hh,v 1.3 2016/03/04 02:22:10 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * Computer: Base class for classes that compute the 3D to 2D value.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef Computer_H
#define Computer_H

#include <iostream>
#include <string>

#include <Mdv/MdvxField.hh>

using namespace std;


class Computer
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  Computer(const string &field_name_prefix);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~Computer();


  /*********************************************************************
   * computeField() - Compute the 2D field based on the given 3D field.
   *
   * Returns a pointer to the new 2D field on success, 0 on failure.
   */

  MdvxField *computeField(const MdvxField &field_3d);
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  string _fieldNamePrefix;   // Set by derived classes
  
  fl32 _badDataValue;
  fl32 _missingDataValue;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _create2DField() - Create the blank 2D field based on the given
   *                    3D field.
   *
   * Returns a pointer to the new 2D field on success, 0 on failure.
   */

  MdvxField *_create2DField(const MdvxField &field_3d) const;
  

  ///////////////////////////////
  // Protected virtual methods //
  ///////////////////////////////

  /*********************************************************************
   * _initCompute() - Initialize the computation for this data column.
   */

  virtual void _initCompute() = 0;
  

  /*********************************************************************
   * _addValue() - Add the given value to the current column computation.
   */

  virtual void _addValue(const fl32 data_value) = 0;
  

  /*********************************************************************
   * _computeValue() - Compute the final 2D value.
   *
   * Returns the value computed.
   */

  virtual fl32 _computeValue() = 0;
  

};

#endif
