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
/////////////////////////////////////////////////////////////
// BasinField.hh
//
// Class representing a field in the database information about
// a basin.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2000
//
///////////////////////////////////////////////////////////////

#ifndef BasinField_H
#define BasinField_H

#include <iostream>
#include <string>

#include <shapelib/shapefil.h>
using namespace std;



class BasinField
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  typedef enum
  {
    TYPE_INTEGER,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_INVALID
  }
  field_type_t;
  

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  // constructor

  BasinField(const DBFHandle dbf_handle,
	     const int shape_num,
	     const int field_num,
	     const bool debug_flag = false);

  // destructor
  
  ~BasinField();


  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Print the current basin field information to the given stream for
  // debugging purposes.

  void print(ostream &stream) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // Get the field number of the field

  int getFieldNumber(void) const
  {
    return _fieldNumber;
  }
  
  // Get the field type

  field_type_t getFieldType(void) const
  {
    return _fieldType;
  }
  
  // Get the title of the field

  string getTitle(void) const
  {
    return _title;
  }
  
  // Get the width of the field

  int getWidth(void) const
  {
    return _width;
  }
  
  // Get the number of decimal places in the field

  int getDecimals(void) const
  {
    return _decimals;
  }
  
  // Get the value for the field.  Call getFieldType() first to
  // determine which of these methods to call.  Otherwise, you'll
  // get a bad default value for the field.

  int getIntegerValue(void) const
  {
    return _intValue;
  }
  
  double getDoubleValue(void) const
  {
    return _doubleValue;
  }
  
  string getStringValue(void) const
  {
    return _stringValue;
  }
  
  // Get the field value as a string.

  string getValueAsString(void) const;
  

protected:
  
private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _debugFlag;
  
  // Information about the field

  int _fieldNumber;
  field_type_t _fieldType;
  string _title;
  int _width;
  int _decimals;

  // The actual field value, which to use depends on the value of
  // _fieldType

  int _intValue;
  double _doubleValue;
  string _stringValue;


  /////////////////////
  // Private methods //
  /////////////////////

  // Convert the given field type value to a string for printing.

  string _fieldTypeToString(field_type_t field_type) const;
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("BasinField");
  }
  
};

#endif
