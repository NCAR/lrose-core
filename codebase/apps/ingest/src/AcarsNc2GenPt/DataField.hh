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
 *   $Date: 2016/03/07 01:22:59 $
 *   $Id: DataField.hh,v 1.2 2016/03/07 01:22:59 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DataField: Class for storing the data field information.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DataField_H
#define DataField_H

#include <netcdf.hh>
#include <string>

#include "Converter.hh"

using namespace std;


class DataField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  DataField(const string &nc_field_name,
	    const string &genpt_field_name,
	    const string &genpt_units,
	    Converter *converter,
	    const bool debug_flag = false);
  
  
  /*********************************************************************
   * Destructor
   */

  virtual ~DataField();
  

  /*********************************************************************
   * getData() - Retrieve the data for this field from the given netCDF
   *             file.
   */

  void getData(NcFile &nc_file,
	       const string &missing_data_value_att_name);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /*********************************************************************
   * getDataValue() - Get the indicated data value from the data array.
   *
   * Returns true if the data was successfully retrieved and didn't have
   * a missing data value, false otherwise.
   */

  bool getDataValue(const int index,
		    double &data_value) const
  {
    if (!_dataRead)
      return false;
    
    data_value = _fieldValues->as_float(index);
    
    if (data_value == _missingDataValue)
      return false;
    
    return true;
  }
  

  /*********************************************************************
   * getGenptName()
   */

  string getGenptName() const
  {
    return _genptFieldName;
  }
  

  /*********************************************************************
   * getGenptUnits()
   */

  string getGenptUnits() const
  {
    return _genptUnits;
  }
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const float FLOAT_MISSING_DATA_VALUE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  bool _debug;

  string _ncFieldName;
  string _genptFieldName;
  string _genptUnits;
  
  Converter *_converter;
  
  NcValues *_fieldValues;
  float _missingDataValue;
  bool _dataRead;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _getVarFloatAtt() - Get the specified attribute from the given
   *                     netCDF variable as a float.
   *
   * Returns the attribute value retrieved from the netCDF file on
   * success, the global FLOAT_MISSING_DATA_VALUE on failure.
   */

  float _getVarFloatAtt(const NcVar &variable,
			const string &att_name) const;
  

};

#endif
