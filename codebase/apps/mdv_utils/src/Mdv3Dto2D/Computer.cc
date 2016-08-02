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
//   $Date: 2016/03/04 02:22:10 $
//   $Id: Computer.cc,v 1.3 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Computer: Base class for classes that compute the 3D to 2D value.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <toolsa/str.h>

#include "Computer.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

Computer::Computer(const string &field_name_prefix) :
  _fieldNamePrefix(field_name_prefix)
{
}

  
/*********************************************************************
 * Destructor
 */

Computer::~Computer()
{
}


/*********************************************************************
 * computeField() - Compute the 2D field based on the given 3D field.
 *
 * Returns a pointer to the new 2D field on success, 0 on failure.
 */

MdvxField *Computer::computeField(const MdvxField &field_3d)
{
  static const string method_name = "Computer::computeField()";
  
  // Create the blank 2D field

  MdvxField *field_2d;
  
  if ((field_2d = _create2DField(field_3d)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating blank 2D field" << endl;
    
    return 0;
  }
  
  // Calculate each of the 2D values

  Mdvx::field_header_t field_hdr_3d = field_3d.getFieldHeader();
  int plane_size = field_hdr_3d.nx * field_hdr_3d.ny;
  
  fl32 *data_2d = (fl32 *)field_2d->getVol();
  fl32 *data_3d = (fl32 *)field_3d.getVol();
  
  _badDataValue = field_hdr_3d.bad_data_value;
  _missingDataValue = field_hdr_3d.missing_data_value;
  
  for (int i = 0; i < plane_size; ++i)
  {
    // Initialize the computation

    _initCompute();
    
    // Add each element in the column to the computation

    for (int z = 0; z < field_hdr_3d.nz; ++z)
      _addValue(data_3d[(z * plane_size) + i]);
    
    // Finish the computation

    data_2d[i] = _computeValue();
      
  } /* endfor - i */
  
  return field_2d;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _create2DField() - Create the blank 2D field based on the given
 *                    3D field.
 *
 * Returns a pointer to the new 2D field on success, 0 on failure.
 */

MdvxField *Computer::_create2DField(const MdvxField &field_3d) const
{
  // Create the field header for the 2D field

  Mdvx::field_header_t field_hdr_2d = field_3d.getFieldHeader();

  field_hdr_2d.nz = 1;
  field_hdr_2d.volume_size = field_hdr_2d.nx * field_hdr_2d.ny *
    field_hdr_2d.data_element_nbytes;
  field_hdr_2d.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr_2d.dz_constant = 1;
  field_hdr_2d.data_dimension = 2;
  field_hdr_2d.min_value = 0.0;
  field_hdr_2d.max_value = 0.0;
  
  string field_name;
  field_name = _fieldNamePrefix + field_hdr_2d.field_name_long;
  STRcopy(field_hdr_2d.field_name_long, field_name.c_str(),
	  MDV_LONG_FIELD_LEN);

  field_name = _fieldNamePrefix + field_hdr_2d.field_name;
  STRcopy(field_hdr_2d.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  
  // Create the vlevel header for the new field

  Mdvx::vlevel_header_t vlevel_hdr_2d;
  memset(&vlevel_hdr_2d, 0, sizeof(vlevel_hdr_2d));
  
  vlevel_hdr_2d.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr_2d.level[0] = 0.5;
  
  // Now create and return the new field

  return new MdvxField(field_hdr_2d, vlevel_hdr_2d, (void *)0, true);
}

		       
