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
/**
 *
 * @file SdsDataField.cc
 *
 * @class SdsDataField
 *
 * Base class for a data field that is stored in the SDS section of a
 * TRMM file.
 *  
 * @date 10/31/2008
 *
 */

#include <iostream>
#include <string.h>

#include "SdsDataField.hh"

using namespace std;

const int SdsDataField::MAX_FIELD_NAME_LEN = 64;

const double SdsDataField::MDV_BAD_DATA_VALUE = -88888.0;
const double SdsDataField::MDV_MISSING_DATA_VALUE = -99999.0;


/*********************************************************************
 * Constructors
 */

SdsDataField::SdsDataField(const string &sds_field_name,
			   const string &prog_name,
			   const string &output_url,
			   const bool debug_flag,
			   const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _progName(prog_name),
  _sdsFieldName(sds_field_name),
  _outputUrl(output_url)
{
}


/*********************************************************************
 * Destructor
 */

SdsDataField::~SdsDataField()
{
}


/*********************************************************************
 * createMdvFiles()
 */

bool SdsDataField::createMdvFiles(HdfFile &hdf_file)
{
  static const string method_name = "SdsDataField::createMdvFiles()";
  
  if (_debug)
    cerr << "Processing field " << _sdsFieldName << endl;
  
  // Read the data from the TRMM file

  float64 *raw_data;
  vector< int > dimensions;
  string field_long_name;
  string field_units;
  double missing_data_value;
  
  if ((raw_data = hdf_file.readSdsData(_sdsFieldName.c_str(),
				       dimensions,
				       field_long_name, field_units,
				       missing_data_value)) == 0)
    return false;
  
  // Check the dimensions of the data

  if (!_checkDimensions(hdf_file.getNumLats(), hdf_file.getNumLons(),
			dimensions))
  {
    delete [] raw_data;
    return false;
  }
  
  // Create the MDV files

  if (!_createMdvFiles(hdf_file.getFilePath(),
		       raw_data, hdf_file.getProj(), dimensions,
		       field_long_name, field_units, missing_data_value))
  {
    delete [] raw_data;
    return false;
  }
  
  // Clean up memory

  delete [] raw_data;
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
