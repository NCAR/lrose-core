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
 *   $Date: 2016/03/07 01:23:05 $
 *   $Id: InputVar.hh,v 1.3 2016/03/07 01:23:05 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * InputVar: Class for handling an input variable.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef InputVar_H
#define InputVar_H

#include <string>

#include <Mdv/MdvxField.hh>
#include <toolsa/MsgLog.hh>

#include "NetcdfDataset.hh"
#include "NetcdfUtils.hh"

using namespace std;


class InputVar
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  InputVar(const string &nc_var_name,
	   const string &missing_data_attr_name,
	   const string &units_attr_name,
	   const string &mdv_field_name,
	   const double mdv_missing_data_value,
	   const bool specify_mdv_scaling,
	   const double mdv_scale,
	   const double mdv_bias,
	   const bool debug_flag,
	   MsgLog *msg_log);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~InputVar();


  ////////////////////
  // Input Methodds //
  ////////////////////

  /*********************************************************************
   * readData() - Read the data for this variable from the given netCDF
   *              file.
   *
   * Returns true on success, false on failure.
   *
   * Note that this method assumes that the caller has already loaded all
   * of the data for this variable into the given nc_dataset object.
   */

  bool readData(NetcdfDataset *nc_dataset,
		const int num_input_rows, const int num_input_cols);
  

  /*********************************************************************
   * createMdvxField() - Create an MdvxField object using the information
   *                     from this field.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  MdvxField *createMdvxField(const MdvxPjg &projection,
			     time_t data_time,
			     const fl32 *lat_data,
			     const fl32 *lon_data) const;
  

  /*********************************************************************
   * remapInput() - Remap the input netCDF data to the output MDV field.
   */
  
  bool remapInput(MdvxField *field,
		  const fl32 *lat_data, const fl32 *lon_data) const;
  

  ////////////////////
  // Access Methods //
  ////////////////////

  /*********************************************************************
   * GetMsgLog() - Get the message log.  This method is required for use
   *               of the POSTMSG macro.
   */

  MsgLog &GetMsgLog()
  {
    return *_msgLog;
  }


  /*********************************************************************
   * getNcVarName() - Get the netCDF file variable name.
   */

  string getNcVarName() const
  {
    return _ncVarName;
  }


protected:

  ///////////////////////
  // Protected Members //
  ///////////////////////

  bool _debug;
  MsgLog *_msgLog;

  string _ncVarName;
  string _missingDataAttrName;
  string _unitsAttrName;
  
  string _mdvFieldName;
  double _mdvMissingDataValue;
  string _mdvUnits;
  bool _specifyMdvScaling;
  double _mdvScale;
  double _mdvBias;
  
  int _numInputRows;
  int _numInputCols;
  
  fl32 *_fieldData;
  

  ///////////////////////
  // Protected Methods //
  ///////////////////////

  /*********************************************************************
   * _createBlankMdvxField() - Create an empty MdvxField object.
   *
   * Returns a pointer to the created field on success, 0 on failure.
   */

  static MdvxField *_createBlankMdvxField(const MdvxPjg &projection,
					  time_t data_time,
					  fl32 bad_data_value,
					  fl32 missing_data_value,
					  const string &field_name,
					  const string &field_name_long,
					  const string &units);
  

};

#endif
