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
 *   $Id: FirstOrderTrender.hh,v 1.2 2016/03/06 23:15:37 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FirstOrderTrender: Class that trends a field using first-order trending.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2002
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FirstOrderTrender_HH
#define FirstOrderTrender_HH

#include <Mdv/MdvxField.hh>

#include "FieldTrender.hh"


class FirstOrderTrender : public FieldTrender
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

  FirstOrderTrender();
  

  /*********************************************************************
   * Destructor
   */

  virtual ~FirstOrderTrender(void);
  

  //////////////////////
  // Trending methods //
  //////////////////////

  /*********************************************************************
   * updateField() - Put the given field into the trending list as the
   *                 current value of the field.
   */

  virtual void updateField(MdvxField &current_field);
  

  /*********************************************************************
   * createTrendedField() - Create a trended field using the field
   *                        information currently stored in the trender.
   *
   * Returns a pointer to the created trended field on success, or
   * 0 if there was an error.
   *
   * Note that the calling method is responsible for deleting the
   * returned pointer when it is no longer needed.
   */

  virtual MdvxField *createTrendedField(void);
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  MdvxField *_prevField;
  MdvxField *_currField;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _createTrendedField() - Create the trended field object.  Note that
   *                         the returned field object will contain all
   *                         missing data values.
   *
   * Returns a pointer to the created field, or 0 if the field could
   * not be created for some reason.
   */

  MdvxField *_createTrendedField(void) const;
  

};


#endif
