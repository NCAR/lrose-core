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
 *   $Id: InterpField.hh,v 1.4 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * InterpField: Base class for interpolated fields.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef InterpField_H
#define InterpField_H

#include <iostream>
#include <string>

#include <Mdv/Mdvx.hh>

#include "Interpolater.hh"

using namespace std;


class InterpField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   *
   * Note that this object takes control of the Interpolater pointer and
   * deletes the pointer in the destructor.
   */

  InterpField(const string &field_name,
	      const string &field_units,
	      Interpolater *interpolater,
	      const bool output_field_flag = true,
	      const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~InterpField();


  /*********************************************************************
   * init() - Initialize all of the accumulation grids so we can start a
   *          new interpolation.
   *
   * Returns true on success, false on failure.
   */

  virtual bool init();


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

  virtual fl32 getMissingDataValue() const = 0;
  

  /*********************************************************************
   * interpolate() - Interpolate the accumulation grids.
   *
   * Returns true on success, false on failure.
   */

  virtual bool interpolate(const fl32 bad_data_value);
  

  /*********************************************************************
   * getInterpolation() - Get the interpolated grid.
   *
   * Returns a pointer to the interpolation grid.
   *
   * Note that this pointer points to static memory and must not be
   * changed or deleted by the caller.
   */

  virtual fl32 *getInterpolation()
  {
    return _interpGrid;
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
  
  Interpolater *_interpolater;
  
  bool _outputField;
  
  fl32 *_interpGrid;
  
  Mdvx::encoding_type_t _encodingType;
  Mdvx::scaling_type_t _scalingType;
  double _scale;
  double _bias;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _addObs() - Add the given observation to the intepolated field.
   *
   * Returns true on success, false on failure.
   */

  bool _addObs(const double obs_value,
	       const double obs_lat,
	       const double obs_lon,
	       const double bad_obs_value,
	       const float *interp_distance_grid);


};

#endif
