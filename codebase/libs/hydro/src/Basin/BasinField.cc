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
///////////////////////////////////////////////////////////////
// BasinField.cc
//
// Class representing a field in the database information about
// a basin.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <string>

#include <hydro/BasinField.hh>
#include <shapelib/shapefil.h>
using namespace std;


/*********************************************************************
 * Constructors
 */

BasinField::BasinField(const DBFHandle dbf_handle,
		       const int shape_num,
		       const int field_num,
		       const bool debug_flag) :
  _debugFlag(debug_flag),
  _fieldNumber(field_num)
{
  // Get the field information from the file

  char field_name[BUFSIZ];

  DBFFieldType field_type = DBFGetFieldInfo(dbf_handle,
					    field_num,
					    field_name,
					    &_width,
					    &_decimals);
  
  // Convert to internal format where necessary

  _title = field_name;
  
  switch (field_type)
  {
  case FTString :
    _fieldType = TYPE_STRING;
    _stringValue = DBFReadStringAttribute(dbf_handle, shape_num, field_num);
    break;
    
  case FTInteger :
    _fieldType = TYPE_INTEGER;
    _intValue = DBFReadIntegerAttribute(dbf_handle, shape_num, field_num);
    break;

  case FTDouble :
    _fieldType = TYPE_DOUBLE;
    _doubleValue = DBFReadDoubleAttribute(dbf_handle, shape_num, field_num);
    break;
    
  case FTInvalid :
    _fieldType = TYPE_INVALID;
    break;
  } /* endswitch - field_type */
  
}


/*********************************************************************
 * Destructor
 */

BasinField::~BasinField()
{
  // Do nothing
}


/*********************************************************************
 * getValueAsString() - Get the field value as a string.
 */

string BasinField::getValueAsString(void) const
{
  char value_string[BUFSIZ];
  
  switch (_fieldType)
  {
  case TYPE_INTEGER :
    sprintf(value_string, "%d", _intValue);
    return value_string;
      
  case TYPE_DOUBLE :
    sprintf(value_string, "%f", _doubleValue);
    return value_string;
      
  case TYPE_STRING :
    return _stringValue;
      
  case TYPE_INVALID :
    return "** INVALID **";
  } /* endswitch - _fieldType */
    
  return "** INVALID **";
}


/*********************************************************************
 * print() - Print the current basin field information to the given stream
 *           for debugging purposes.
 */

void BasinField::print(ostream &stream) const
{
  cerr << "field number = " << _fieldNumber << endl;
  cerr << "field type = " << _fieldTypeToString(_fieldType) << endl;
  cerr << "title = " << _title << endl;
  cerr << "width = " << _width << endl;
  cerr << "decimals = " << _decimals << endl;
  
  cerr << "value = ";
  
  switch (_fieldType)
  {
  case TYPE_INTEGER :
    cerr << _intValue;
    break;
    
  case TYPE_DOUBLE :
    cerr << _doubleValue;
    break;
    
  case TYPE_STRING :
    cerr << _stringValue;
    break;
    
  case TYPE_INVALID :
    cerr << "*** INVALID ***";
    break;
  } /* endswitch - _fieldType */

  cerr << endl;
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/*********************************************************************
 * _fieldTypeToString() - Convert the given field type value to a string
 *                        for printing.
 */

string BasinField::_fieldTypeToString(field_type_t field_type) const
{
  switch (field_type)
  {
  case TYPE_INTEGER :
    return "Integer";
    
  case TYPE_DOUBLE :
    return "Double";
    
  case TYPE_STRING :
    return "String";
    
  case TYPE_INVALID :
    return "INVALID";
  } /* endswitch - field_type */
  
  return "INVALID";
}
