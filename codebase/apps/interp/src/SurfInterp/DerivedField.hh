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
 *   $Id: DerivedField.hh,v 1.3 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DerivedField: Base class for derived fields.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DerivedField_H
#define DerivedField_H

#include <iostream>
#include <map>
#include <string>

#include <dataport/port_types.h>
#include <euclid/Pjg.hh>

#include "DerivedField.hh"
#include "StnInterpField.hh"

using namespace std;


class DerivedField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  DerivedField(const string &field_name,
	       const string &field_units,
	       const Pjg &output_proj,
	       const bool output_field_flag = true,
	       const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~DerivedField();


  /*********************************************************************
   * derive() - Calculate the derivation.
   *
   * Returns true on success, false on failure.
   */

  virtual bool derive(map< int, StnInterpField* > &interp_field_list,
		      map< int, DerivedField* > &der_field_list) = 0;
  

  /*********************************************************************
   * getDerivation() - Get the derived grid.
   *
   * Returns a pointer to the derived grid.
   *
   * Note that this pointer points to static memory and must not be
   * changed or deleted by the caller.
   */

  virtual fl32 *getDerivation()
  {
    return _derivedGrid;
  }
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getFieldName() - Get the field name.
   */

  virtual string getFieldName() const
  {
    return _fieldName;
  }
  

  /*********************************************************************
   * getFieldUnits() - Get the field units.
   */

  virtual string getFieldUnits() const
  {
    return _fieldUnits;
  }
  

  /*********************************************************************
   * getMissingDataValue() - Get the missing data value.
   */

  virtual fl32 getMissingDataValue() const
  {
    return _missingDataValue;
  }
  

  /*********************************************************************
   * setOutputFlag() - Set the output flag to the given value.
   */

  virtual void setOutputFlag(const bool output_flag)
  {
    _outputField = output_flag;
  }
  

  /*********************************************************************
   * isOutput() - Tells whether this is a field to output or not.
   */

  virtual bool isOutput() const
  {
    return _outputField;
  }
  

  /*********************************************************************
   * setEncodingType() - Set the output encoding type to the given value.
   */

  virtual void setEncodingType(const Mdvx::encoding_type_t encoding_type)
  {
    _encodingType = encoding_type;
  }
  

  /*********************************************************************
   * getEncodingType() - Get the output encoding type.
   */

  virtual Mdvx::encoding_type_t getEncodingType() const
  {
    return _encodingType;
  }
  

  /*********************************************************************
   * setScaling() - Set the output scaling to the given values.
   */

  virtual void setScaling(const Mdvx::scaling_type_t scaling_type,
			  const double scale,
			  const double bias)
  {
    _scalingType = scaling_type;
    _scale = scale;
    _bias = bias;
  }
  

  /*********************************************************************
   * getScalingType() - Get the output scaling type.
   */

  virtual Mdvx::scaling_type_t getScalingType() const
  {
    return _scalingType;
  }
  

  /*********************************************************************
   * getScale() - Get the output scale value.
   */

  virtual double getScale() const
  {
    return _scale;
  }
  

  /*********************************************************************
   * getBias() - Get the output bias value.
   */

  virtual double getBias() const
  {
    return _bias;
  }
  

protected:
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;
  
  string _fieldName;
  string _fieldUnits;
  
  bool _outputField;
  
  Pjg _outputProj;
  fl32 *_derivedGrid;
  fl32 _missingDataValue;
  Mdvx::encoding_type_t _encodingType;
  Mdvx::scaling_type_t _scalingType;
  double _scale;
  double _bias;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _getFieldData() - Get the data for the given field from the given
   *                   field list.
   *
   * Returns a pointer to the static data on success, 0 on failure.
   */

  fl32 *_getFieldData(map< int, StnInterpField* > &interp_field_list,
		      const int field_key,
		      fl32 &missing_value) const;

  
  fl32 *_getFieldData(map< int, DerivedField* > &der_field_list,
		      const int field_key,
		      fl32 &missing_value) const;

  
};

#endif
