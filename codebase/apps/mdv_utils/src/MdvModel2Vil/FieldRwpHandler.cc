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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/04 02:22:11 $
//   $Id: FieldRwpHandler.cc,v 1.3 2016/03/04 02:22:11 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FieldRwpHandler: Class that supplies the RWP field by reading it
 *                  directly from the input MDV file.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "FieldRwpHandler.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

FieldRwpHandler::FieldRwpHandler(const string &url,
				 const DateTime &gen_time,
				 const DateTime &fcst_time) :
  RwpHandler(url, gen_time, fcst_time),
  _rwpField(0)
{
}

FieldRwpHandler::FieldRwpHandler(const string &url,
				 const DateTime &time_centroid) :
  RwpHandler(url, time_centroid),
  _rwpField(0)
{
}

  
/*********************************************************************
 * Destructor
 */

FieldRwpHandler::~FieldRwpHandler()
{
}


/*********************************************************************
 * init() - Initialize the data.
 *
 * Returns true on success, false on failure.
 */

bool FieldRwpHandler::init(const string &rwp_field_name)
{
  static const string method_name = "FieldRwpHandler::init()";
  
  // Read in the input file.

  vector< string > input_field_names;
  
  input_field_names.push_back(rwp_field_name);
  
  if (!_readInputData(input_field_names))
    return false;
  
  // Get a pointers to the input field.

  _rwpField = _mdvx.getField(0);
  
  return true;
}


/*********************************************************************
 * getRwpField() - Get the RWP field.
 *
 * Returns a pointer to the field on success, 0 on failure.  The pointer
 * is then owned by the calling method and must be deleted there.
 */

MdvxField *FieldRwpHandler::getRwpField()
{
  return new MdvxField(*_rwpField);
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
