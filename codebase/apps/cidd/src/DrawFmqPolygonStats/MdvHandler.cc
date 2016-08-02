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
 * @file MdvHandler.cc
 *
 * @class MdvHandler
 *
 * Class for handling MDV file input.
 *  
 * @date 3/18/2009
 *
 */

#include <Mdv/MdvxRadar.hh>

#include "MdvHandler.hh"

using namespace std;


/*********************************************************************
 * Constructor
 */

MdvHandler::MdvHandler(const bool debug_flag) :
  _debug(debug_flag),
  _url(""),
  _searchMarginSecs(0),
  _latestRequestTime(0),
  _useFieldNames(true),
  _readThresholdField(false)
{
}


/*********************************************************************
 * Destructor
 */

MdvHandler::~MdvHandler()
{
}


/*********************************************************************
 * init()
 */

bool MdvHandler::init(const string &url, const int search_margin_secs,
		      const bool use_field_names,
		      const bool debug_flag)
{
  // Save the parameters

  _debug = debug_flag;
  
  _url = url;
  _searchMarginSecs = search_margin_secs;
  _useFieldNames = use_field_names;
  
  return true;
}


/*********************************************************************
 * setDbzField()
 */

bool MdvHandler::setDbzField(const string &field_name,
			     const int field_num)
{
  static const string method_name = "MdvHandler::setDbzField()";
  
  _dbzFieldInfo.setFieldName(field_name);
  _dbzFieldInfo.setFieldNum(field_num);
  _dbzFieldInfo.setIsLog(true);
  
  _readFieldNames.insert(field_name);
  _readFieldNums.insert(field_num);
  
  return true;
}


/*********************************************************************
 * setZdrField()
 */

bool MdvHandler::setZdrField(const string &field_name,
			     const int field_num)
{
  static const string method_name = "MdvHandler::setZdrField()";
  
  _zdrFieldInfo.setFieldName(field_name);
  _zdrFieldInfo.setFieldNum(field_num);
  _zdrFieldInfo.setIsLog(true);
  
  _readFieldNames.insert(field_name);
  _readFieldNums.insert(field_num);
  
  return true;
}


/*********************************************************************
 * addStatisticField()
 */

bool MdvHandler::addStatisticField(const string &field_name,
				   const int field_num,
				   const bool is_log)
{
  static const string method_name = "MdvHandler::addStatisticField()";
  
  _statFieldInfo.push_back(FieldInfo(field_name, field_num, is_log));

  _readFieldNames.insert(field_name);
  _readFieldNums.insert(field_num);
  
  return true;
}


/*********************************************************************
 * addDiscreteStatisticField()
 */

bool MdvHandler::addDiscreteStatisticField(const string &field_name,
					   const int field_num)
{
  static const string method_name = "MdvHandler::addDiscreteStatisticField()";
  
  _discreteFieldInfo.push_back(FieldInfo(field_name, field_num, false));

  _readFieldNames.insert(field_name);
  _readFieldNums.insert(field_num);
  
  return true;
}


/*********************************************************************
 * addDropsizeThreshField()
 */

bool MdvHandler::addDropsizeThreshField(const string &field_name,
					const int field_num)
{
  static const string method_name = "MdvHandler::addDropsizeThreshField()";
  
  _dropsizeThreshFieldInfo.push_back(FieldInfo(field_name, field_num));

  _readFieldNames.insert(field_name);
  _readFieldNums.insert(field_num);
  
  return true;
}


/*********************************************************************
 * addThresholdField()
 */

bool MdvHandler::addThresholdField(const string &field_name,
				   const int field_num)
{
  static const string method_name = "MdvHandler::addThresholdField()";
  
  _thresholdFieldInfo.push_back(FieldInfo(field_name, field_num));

  _readFieldNames.insert(field_name);
  _readFieldNums.insert(field_num);
  
  return true;
}


/*********************************************************************
 * getData()
 */

bool MdvHandler::getData(const time_t data_time)
{
  if (_latestRequestTime != 0 &&
      _latestRequestTime == data_time)
    return true;
  
  return _readFile(data_time);
}


/*********************************************************************
 * getScanMode()
 */

int MdvHandler::getScanMode() const
{
  static const string method_name = "MdvHandler::getScanMode()";
  
  // Retrieve the radar information from the MDV file

  MdvxRadar radar_info;
  
  if (radar_info.loadFromMdvx(_mdvx) != 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Unable to get radar information from MDV file" << endl;
    
    return DS_RADAR_UNKNOWN_MODE;
  }
  
  if (!radar_info.radarParamsAvail())
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No radar information in the MDV file" << endl;
    
    return DS_RADAR_UNKNOWN_MODE;
  }
  
  return radar_info.getRadarParams().scanMode;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _readFile()
 */

bool MdvHandler::_readFile(const time_t data_time)
{
  static const string method_name = "MdvHandler::_readFile()";

  if (_debug)
    cerr << "**** Reading MDV data for time: " << DateTime::str(data_time) << endl;
  
  // Clear out the old information

  _latestRequestTime = 0;

  // Set up the read request

  _mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   _url, _searchMarginSecs,
		   data_time);

  if (_useFieldNames)
  {
    for (set< string >::const_iterator field_name = _readFieldNames.begin();
	 field_name != _readFieldNames.end(); ++field_name)
      _mdvx.addReadField(*field_name);
  }
  else
  {
    for (set< int >::const_iterator field_num = _readFieldNums.begin();
	 field_num != _readFieldNums.end(); ++field_num)
      _mdvx.addReadField(*field_num);
  }
  
  _mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading MDV data from URL: " << _url << endl;
    cerr << _mdvx.getErrStr() << endl;
    
    return false;
  }
  
  // Go through the fields and make sure that all of the projections
  // match.  If they don't then we can't calculate the statistics.
  // We also must have a cartesian projection for the calculations.

  Mdvx::field_header_t base_field_hdr = _mdvx.getField(0)->getFieldHeader();
  Mdvx::vlevel_header_t base_vlevel_hdr = _mdvx.getField(0)->getVlevelHeader();
  
  if (base_field_hdr.proj_type != Mdvx::PROJ_LATLON &&
      base_field_hdr.proj_type != Mdvx::PROJ_FLAT)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "MDV file contains data using the "
	 << Mdvx::projType2Str(base_field_hdr.proj_type)
	 << " projection" << endl;
    cerr << "We can only calculate statistics using a cartesian "
	 << "projection (FLAT or LATLON)" << endl;
    
    return false;
  }
  
  for (int i = 1; i < _mdvx.getNFields(); ++i)
  {
    Mdvx::field_header_t field_hdr = _mdvx.getField(i)->getFieldHeader();
    
    cerr << "===> " << field_hdr.field_name << endl;
    
    if (field_hdr.proj_type != base_field_hdr.proj_type ||
	field_hdr.vlevel_type != base_field_hdr.vlevel_type ||
	field_hdr.nx != base_field_hdr.nx ||
	field_hdr.ny != base_field_hdr.ny ||
	field_hdr.nz != base_field_hdr.nz ||
	field_hdr.grid_minx != base_field_hdr.grid_minx ||
	field_hdr.grid_miny != base_field_hdr.grid_miny ||
	field_hdr.grid_dx != base_field_hdr.grid_dx ||
	field_hdr.grid_dy != base_field_hdr.grid_dy)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Fields in MDV file don't use matching projections." << endl;
      cerr << "Cannot process data" << endl;
      
      return false;
    }
    
    Mdvx::vlevel_header_t vlevel_hdr = _mdvx.getField(i)->getVlevelHeader();
    
    for (int z = 0; z < base_field_hdr.nz; ++z)
    {
      if (vlevel_hdr.type[i] != base_vlevel_hdr.type[i] ||
	  vlevel_hdr.level[i] != base_vlevel_hdr.level[i])
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Fields in MDV file don't use matching vertical levels" << endl;
	cerr << "Cannot process data" << endl;
	
	return false;
      }
    } /* endfor - z */
    
  } /* endfor i */
  
  // Save pointers to the fields for processing

  _dbzFieldInfo.setField(_findField(_dbzFieldInfo.getFieldName(),
				    _dbzFieldInfo.getFieldNum()));
  _zdrFieldInfo.setField(_findField(_zdrFieldInfo.getFieldName(),
				    _zdrFieldInfo.getFieldNum()));
  
  for (vector< FieldInfo >::iterator field_info = _statFieldInfo.begin();
       field_info != _statFieldInfo.end(); ++field_info)
  {
    MdvxField *field = _findField(field_info->getFieldName(),
				  field_info->getFieldNum());
    
    field_info->setField(field);
  } /* endfor - field_info */
  
  for (vector< FieldInfo >::iterator field_info = _discreteFieldInfo.begin();
       field_info != _discreteFieldInfo.end(); ++field_info)
  {
    MdvxField *field = _findField(field_info->getFieldName(),
				  field_info->getFieldNum());
    
    field_info->setField(field);
  } /* endfor - field_info */
  
  for (vector< FieldInfo >::iterator field_info = _thresholdFieldInfo.begin();
       field_info != _thresholdFieldInfo.end(); ++field_info)
  {
    MdvxField *field = _findField(field_info->getFieldName(),
				  field_info->getFieldNum());
    
    field_info->setField(field);
  } /* endfor - field_info */
  
  for (vector< FieldInfo >::iterator field_info =
	 _dropsizeThreshFieldInfo.begin();
       field_info != _dropsizeThreshFieldInfo.end(); ++field_info)
  {
    MdvxField *field = _findField(field_info->getFieldName(),
				  field_info->getFieldNum());
    
    field_info->setField(field);
  } /* endfor - field_info */
  
  // Save the latest request time so we know when we need to request new data

  _latestRequestTime = data_time;
  
  return true;
}
