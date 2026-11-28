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
/*
 *  $Id: OutputFile.cc,v 1.10 2018/01/26 18:43:35 jcraig Exp $
 *
 */

# ifndef    lint
static char RCSid[] = "$Id: OutputFile.cc,v 1.10 2018/01/26 18:43:35 jcraig Exp $";
# endif     /* not lint */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	OutputFile
//
// Author:	G. M. Cunning
//
// Date:	Wed Jul 25 20:59:10 2007
//
// Description: 
//
//


// C++ include files
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

// System/RAP include files
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <euclid/Pjg.hh>
#include <toolsa/str.h>

// Local include files
#include "OutputFile.hh"
#include "SeviriData.hh"
#include "Params.hh"

using namespace std;

// define any constants
const string OutputFile::_className    = "OutputFile";

const double OutputFile::MISSING_DATA_VALUE = -9999.0;

template <class T> string toString( T value );

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

OutputFile::OutputFile() :
  _params(0),
  _outMdvx(0),
  _pjg(0),
  _imageTime(-1),
  _lastImageTime(-1),
  _timestamp(-1)
{
  // do nothing
}

OutputFile::OutputFile(const OutputFile &from) :
  _params(0),
  _outMdvx(0),
  _pjg(0),
  _imageTime(-1),
  _lastImageTime(-1),
  _timestamp(-1)
{
  _copy(from);
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
OutputFile::~OutputFile()
{
  delete _outMdvx;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::init
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool 
OutputFile::init(const Params* params)
{
  const string methodName = _className + "::init";

  _params = params;

  _outMdvx = new DsMdvx();

  if (_params->debug_mode != Params::DEBUG_OFF) {
    _outMdvx->setDebug();
    _outMdvx->printWriteOptions(cout);
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::addField
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool
OutputFile::addData(const SeviriData* seviri_data)
{
  const string methodName = _className + "::addData";

  //
  // get the image time to see file should be written
  //
  _imageTime = seviri_data->getImageTime().utime();

  if (_lastImageTime < 0) {

    if (_params->timestamp_mode == Params::TIMESTAMP_INTERNAL) {
      _timestamp = _imageTime;
    }
    else {
      _setTimestamp(seviri_data->getPathName());
    }

    //
    // first seviri_data of pass to process -- setup the projection and
    //   create the MdvxField object
    //    
    _pjg = seviri_data->getProjection();
    assert(_pjg->getProjType() == PjgTypes::PROJ_LATLON);

    _processData(seviri_data);

  }
  else if (_imageTime == _lastImageTime) {

    //
    // one of the channels from the same pass -- create the MdvxField object
    //
    _processData(seviri_data);
									     
  }
  else if (_imageTime != _lastImageTime) {

    if (!write()) {
      cerr << methodName << endl;
      return false;
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::write
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool
OutputFile::write()
{
  const string methodName = _className + "::write";

  //
  // create the master header
  //
  Mdvx::master_header_t masterHdr;
  _setMasterHeader(_timestamp, &masterHdr);
  _outMdvx->setMasterHeader(masterHdr);
  
  //
  // write out set of mdvxField objects added to _outMdvx
  //

  if (_outMdvx->writeToDir(_params->output_url) != 0) {
    cerr << methodName << " -- failed to write file -- " << 
      _outMdvx->getErrStr() << endl;
    return false;
  } 
  else {
    _outMdvx->clearFields();
    _outMdvx->clearChunks();
    _lastImageTime = -1;
    _timestamp = -1;
    _sourceInfo.erase();
  }
 
 return true;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_copy
//
// Description:	take care of making a copy
//
// Returns:	
//
// Notes:
//
//

void 
OutputFile::_copy(const OutputFile& from)
{
  
  if (_outMdvx) {
    _outMdvx = new DsMdvx(*(from._outMdvx));
    _pjg = from._pjg;
    _params = from._params;
    _imageTime = from._imageTime;
    _lastImageTime = from._lastImageTime;
    _timestamp = from._timestamp;
  }
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_remap
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
OutputFile::_remap(MdvxField* field)
{
  const string methodName = _className + "::_remap";

  if(_params->remap_output) {

    MdvxRemapLut lut;

    switch( _params->out_projection_info.type) {
    case Params::PROJ_FLAT:
      field->remap2Flat(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
			_params->out_grid_info.minx, _params->out_grid_info.miny, 
			_params->out_grid_info.dx, _params->out_grid_info.dy, 
			_params->out_projection_info.origin_lat, _params->out_projection_info.origin_lon,
			_params->out_projection_info.rotation);
      break;
	  
    case Params::PROJ_LATLON:
      field->remap2Latlon(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
			  _params->out_grid_info.minx, _params->out_grid_info.miny, 
			  _params->out_grid_info.dx, _params->out_grid_info.dy );
      break;

    case Params::PROJ_LAMBERT_CONF:
      field->remap2Lc2(lut, _params->out_grid_info.nx, _params->out_grid_info.ny, 
		       _params->out_grid_info.minx, _params->out_grid_info.miny, 
		       _params->out_grid_info.dx, _params->out_grid_info.dy,
		       _params->out_projection_info.origin_lat, _params->out_projection_info.origin_lon,
		       _params->out_projection_info.ref_lat_1, _params->out_projection_info.ref_lat_2 );
      break;
    default:
      cerr << methodName << "-- unknown projection; remapping failed." << endl;
    }

  } // endif -- _params->remap_output

}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_setMasterHeader
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void 
OutputFile::_setMasterHeader(const time_t& d_time, 
			     Mdvx::master_header_t *hdr)
{
  memset(hdr, 0, sizeof(hdr));
  
  hdr->time_gen = time((time_t *)NULL);
  hdr->time_begin = d_time;
  hdr->time_end = d_time;
  hdr->time_centroid = d_time;
  hdr->time_expire = d_time + 3600;
  hdr->data_dimension = 2;
  hdr->data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  hdr->native_vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  hdr->vlevel_type = Mdvx::VERT_SATELLITE_IMAGE;
  hdr->vlevel_included = 1;
  hdr->grid_orientation = Mdvx::ORIENT_SN_WE;
  hdr->data_ordering = Mdvx::ORDER_XYZ;
  
  STRcopy(hdr->data_set_info,
	  "Produced by SeviriNc2Mdv", MDV_INFO_LEN);
  STRcopy(hdr->data_set_name, "SEVIRI", MDV_NAME_LEN);
  STRcopy(hdr->data_set_source,
	  _sourceInfo.c_str(), MDV_NAME_LEN);

  
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_setFieldHeader
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void
OutputFile::_setFieldHeader(const string& name, const string& long_name, 
			      const string& units, Mdvx::field_header_t *hdr)
{
  memset(hdr, 0, sizeof(*hdr));

  int nx;
  int ny;
  int nz;
  _pjg->getGridDims(nx, ny, nz);
  hdr->encoding_type = Mdvx::ENCODING_FLOAT32;
  hdr->data_element_nbytes = FLOAT32_SIZE;
  hdr->volume_size = nx*ny*nz*hdr->data_element_nbytes;
  hdr->compression_type = Mdvx::COMPRESSION_NONE;
  hdr->transform_type = Mdvx::DATA_TRANSFORM_NONE;
  hdr->scaling_type = Mdvx::SCALING_NONE;
  hdr->native_vlevel_type = Mdvx::VERT_TYPE_MIXED;
  hdr->vlevel_type = Mdvx::VERT_TYPE_MIXED;
  hdr->dz_constant = 1;
  hdr->nx = nx;
  hdr->ny = ny;
  hdr->nz = nz;
  hdr->proj_type = Mdvx::PROJ_LATLON;
  hdr->proj_origin_lat = _pjg->getOriginLat();
  hdr->proj_origin_lon = _pjg->getOriginLon();
  hdr->proj_rotation = _pjg->getRotation();
  hdr->grid_dx = _pjg->getDx();
  hdr->grid_dy = _pjg->getDy();
  hdr->grid_dz = _pjg->getDz();
  hdr->grid_minx = _pjg->getMinx();
  hdr->grid_miny = _pjg->getMiny();
  hdr->grid_minz = _pjg->getMinz();
  hdr->scale = 1.0;
  hdr->bias = 0.0;
  hdr->bad_data_value = MISSING_DATA_VALUE+1.0;
  hdr->missing_data_value = MISSING_DATA_VALUE;
  
  STRcopy(hdr->field_name_long, long_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(hdr->field_name, name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(hdr->units, units.c_str(), MDV_UNITS_LEN);
  STRcopy(hdr->transform, "none", MDV_TRANSFORM_LEN);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_processData
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool
OutputFile::_processData(const SeviriData* seviri_data)
{
  const string methodName = _className + "::_processData";

  const vector<float*> dataVec = seviri_data->getData();
  const vector<int> bandVec = seviri_data->getBands();
  
  Mdvx::vlevel_header_t vlevelHdr;
  memset(&vlevelHdr, 0, sizeof(Mdvx::vlevel_header_t));
  vlevelHdr.type[0] = Mdvx::VERT_SATELLITE_IMAGE;
  
  for (size_t i = 0; i < dataVec.size(); i++) {

    //
    // create mdvxField object and add to _outMdvx
    //
    Mdvx::field_header_t fieldHdr;
    
    
    string longName = "ch_" + toString(bandVec[i]);
    string name = longName;
    string units = "none";
 
    _getBandInfo(bandVec[i], name, longName, units);

    _setFieldHeader(name, longName, units, &fieldHdr);
    
    MdvxField *field = new MdvxField(fieldHdr, vlevelHdr, static_cast<void*>(dataVec[i]));
    if(_params->remap_output) {
      _remap(field);
      }
    
    field->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_BZIP);

    _outMdvx->addField(field);
  }

  _lastImageTime = _imageTime;

  int idx = seviri_data->getPathName().find_last_of("/") + 1;
  string filename = seviri_data->getPathName().substr(idx) + " ";
  _sourceInfo.append(filename);

  return true;
}

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_getBandInfo
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

void
OutputFile::_getBandInfo(int band_num, string& name, string& long_name, string& units)
{
  const string methodName = _className + "::_getBandInfo";
  
  for (int i = 0; i < _params->band_info_n; i++) {
    if (band_num == _params->_band_info[i].number) {
      name = _params->_band_info[i].short_name;
      long_name = _params->_band_info[i].long_name;
      if (_params->convert_counts) {
	units = _params->_band_info[i].physical_units;
      }
      else {
	units = _params->_band_info[i].count_units;
      }
      return;
   }
  }

  cerr << methodName << " -- did not match a band number to band information provided." << endl;
}



/////////////////////////////////////////////////////////////////////////
//
// Method Name:	OutputFile::_setTimestamp
//
// Description:	create unix time from filename
//
// Returns:	
//
// Notes:	the file name pattern is <dir>/YYYYMMDDHHMM.BB.nc 
//
//

void
OutputFile::_setTimestamp(const string& path)
{

  int startIdx = path.find_last_of("/") + 1;
  string timeStr = path.substr(startIdx, 12) + "00";

  DateTime dateTime(timeStr);
  _timestamp = dateTime.utime();
}



//
// Helper function
//
template <class T>
std::string toString( T value )
{
   ostringstream oss;

   oss << value;

   return oss.str();
}
