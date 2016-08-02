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
 *   $Id: FieldPressHandler.hh,v 1.2 2016/03/04 02:22:11 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FieldPressHandler: Class that gets pressure values from an MdvxField.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FieldPressHandler_H
#define FieldPressHandler_H

#include <Mdv/MdvxField.hh>

#include "PressHandler.hh"

using namespace std;


class FieldPressHandler : public PressHandler
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  FieldPressHandler();
  
  /*********************************************************************
   * Destructor
   */

  virtual ~FieldPressHandler();


  /*********************************************************************
   * init() - Initialize the handler.  Must be called before any other
   *          methods.
   *
   * Returns true on success, false on failure.
   *
   * Note that this method copies the field pointer and assumes that the
   * field continues to exist outside of this object as long as this object
   * is used.
   */

  bool init(MdvxField *pressure_field);
  

  /*********************************************************************
   * getPressureValue() - Get the indicated pressure value.
   *
   * Returns true if the pressure value is available, false otherwise.
   */

  virtual bool getPressureValue(const int plane_index, const int z,
				fl32 &pressure) const;


protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  MdvxField *_pressField;
  Mdvx::field_header_t _pressFieldHdr;
  fl32 *_pressData;
  
};

#endif
