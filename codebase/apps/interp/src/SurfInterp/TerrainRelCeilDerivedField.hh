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
 *   $Date: 2016/03/07 01:50:09 $
 *   $Id: TerrainRelCeilDerivedField.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * TerrainRelCeilDerivedField: Class for calculating the terrain relative
 *                             ceiling derived field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef TerrainRelCeilDerivedField_H
#define TerrainRelCeilDerivedField_H

#include <iostream>
#include <string>

#include "DerivedField.hh"
#include "Terrain.hh"

using namespace std;


class TerrainRelCeilDerivedField : public DerivedField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   *
   * Note that the terrain pointer points to an object owned by the caller
   * and must be deleted by the caller after deleting this object.
   */

  TerrainRelCeilDerivedField(Terrain *terrain,
			     const Pjg &output_proj,
			     const bool output_field_flag = true,
			     const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~TerrainRelCeilDerivedField();


  /*********************************************************************
   * derive() - Calculate the derivation.
   *
   * Returns true on success, false on failure.
   */

  virtual bool derive(map< int, StnInterpField* > &interp_field_list,
		      map< int, DerivedField* > &der_field_list);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////
  
  static const string FIELD_NAME;
  static const string FIELD_UNITS;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  Terrain *_terrain;

  
};

#endif
