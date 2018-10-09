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
 *  $Id: Navigation.cc,v 1.11 2017/11/08 03:02:20 cunning Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	Navigation
//
// Author:	G. M. Cunning
//
// Date:	Wed Jul 27 15:04:38 2011
//
// Description: 
//
//


// C++ include files

// System/RAP include files
#include <toolsa/toolsa_macros.h>
#include <toolsa/umisc.h>
#include <dataport/port_types.h>
#include <Mdv/MdvxField.hh>
// Local include files
#include "Navigation.hh"

using namespace std;

// define any constants
const string Navigation::_className = "Navigation";
const fl32 Navigation::_missingFloat = -9999.0;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Navigation::Navigation(const Params *params) : _params(params)
{
  _isFlipped = false;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Navigation::~Navigation()
{

}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


bool Navigation::getDimensionVarData(const string name,
                                     size_t size,
                                     vector<float>& dataVec,
                                     NcFile &ncf)
{
  const string methodName = _className + string( "::getDimensionVarData" );

  NcVar *nc_var = ncf.get_var(name.c_str());
  if(nc_var == 0) {
    cerr << "ERROR - " << methodName << endl;
    cerr << "  variable missing: " << name << endl;
    return false;
  } 
  else {
    cout << "Processing: " << name << endl;
  }

  dataVec.reserve(size);
  float *dataPtr = (float *) &dataVec[0];
  
  // get variable data
  if (nc_var->get(dataPtr, size) == 0) {
    cerr << "ERROR - " <<  methodName << endl;
    cerr << "  Cannot get " << name  << " data from input netcdf file" << endl;
    return false;
  } 

  // if data is packed, unpack it
  if(nc_var->get_att(_params->netcdf_vattr_scale) != 0)
  {
    double scale = nc_var->get_att(_params->netcdf_vattr_scale)->as_double(0);
    double offset = nc_var->get_att(_params->netcdf_vattr_offset)->as_double(0);
  
    for(size_t i = 0; i < size; i++)
    {
      dataVec[i] = dataVec[i] * scale + offset;
    }
  }

  //  delete nc_var; 
  return true;
}

void Navigation::initCoords(float lat0, float lat1,
                            float lon0, float lon1,
                            float lvl0, float lvl1=NULL)
{
  Mdvx::coord_t coord;
  coord.proj_type = Mdvx::PROJ_LATLON;

  coord.nx = _nLon;
  coord.ny = _nLat;
  coord.nz = _nLvl;

  coord.dx = lon1 - lon0;
  coord.dy = lat1 - lat0;
  if(_nLvl == 1)
  {
    coord.dz = 0;
  }
  else
  {
    coord.dz = lvl1 - lvl0;
  }

  coord.minx = lon0;
  coord.miny = lat0;
  coord.minz = lvl0;

  coord.proj_origin_lon = lat0;
  coord.proj_origin_lat = lon0;

  _proj.init(coord);

}


//////////////////////////////////////////////////////////////////////////
//
// Method Name:	Navigation::initialize
//
// Description:	
//
// Returns:	
//
// Notes: Should not get to this point without a file check. no netcdf testing.
//
//

bool 
Navigation::initialize(NcFile &ncf)
{
  const string methodName = _className + string( "::initialize" );

  vector<float> latDataVector;
  vector<float> lonDataVector;
  vector<float> vlevelDataVector;
  vector<float> fhrDataVector;

  // get dimensions
  _nTime = ncf.get_dim(_params->netcdf_dim_n_time)->size();
  _nLat = ncf.get_dim(_params->netcdf_dim_n_lat)->size();
  _nLon = ncf.get_dim(_params->netcdf_dim_n_lon)->size();
  if(_params->inputDimension == 2)
  {
    _nLvl = 1;
  }else
  {
    _nLvl = ncf.get_dim(_params->netcdf_dim_n_lvl)->size();
  }


  getDimensionVarData("lat", _nLat, latDataVector, ncf);

  // if latitude data is backwards and flipping is not overridden
  if( _params->allowFlip && latDataVector[0] > latDataVector[_nLat-1] )
  {
    _isFlipped = true;
    unsigned int mid = (_nLat+1) / 2;
    float temp;
    for(unsigned int i = 0; i < mid; i++)
    {
      temp = latDataVector[i];
      latDataVector[i] = latDataVector[_nLat - 1 - i];
      latDataVector[_nLat - 1 - i] = temp;
    } 
  }

  getDimensionVarData("lon", _nLon, lonDataVector, ncf);

  // get vertical level info
  if(_params->inputDimension == 3)
  {
    getDimensionVarData("lvl", _nLvl, vlevelDataVector, ncf);

    MEM_zero(_vhdr);
  
    _vhdr.type[0] = Mdvx::VERT_TYPE_PRESSURE;

    for(unsigned int i=0; i < _nLvl; i++)
    {
      _vhdr.level[i] = vlevelDataVector[i];
    }
    // initialize output grid and projection
    initCoords(latDataVector[0], latDataVector[1],
               lonDataVector[0], lonDataVector[1],
               vlevelDataVector[0], vlevelDataVector[1]);
  }else
  {
    _vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
    _vhdr.level[0] = 1000;
  // initialize output grid and projection
  initCoords(latDataVector[0], latDataVector[1],
             lonDataVector[0], lonDataVector[1],
             1000);

  }

  // get forecast hour info
  if(_params->time_source == Params::TIME_FROM_NC_VAR) {
    getDimensionVarData("forc_hrs", _nTime, fhrDataVector, ncf);
    if(_params->roundForcHrs)
    {
      _fhr = (int)(fhrDataVector[0] + 0.5);
    }
    else
    {
      _fhr = fhrDataVector[0];
    }
  }
  
  return true;
}

//////////////////////////////////////////////////////////////////////////
//
// Method Name:	Navigation::cleanup
//
// Description:	
//
// Returns:	
//
// Notes: 
//
//

void 
Navigation::cleanup()
{
  const string methodName = _className + string( "::cleanup" );
/*
  ufree3((void ***) _outputData);
  ufree3((void ***) _distanceError);
  ufree2((void **) _outputLat);
  ufree2((void **) _outputLon);
*/
}

//////////////////////////////////////////////////////////////////////////
//
// Method Name:	Navigation::processNcVar
//
// Description:	
//
// Returns:	
//
// Notes:
//
//

bool 
Navigation::processNcVar(const string &fieldName,
			 const string &fieldNameLong,
			 const string &units,
			 time_t dateTime,
			 const NcVar* nc_var, 
			 MdvxField** mdvx_field)
{
  const string methodName = _className + string( "::_processNcVar" );
  
  size_t totalSize = _nTime * _nLvl * _nLat * _nLon;

  // get data from nc_var is tested prior to method call
  vector<float> dataVector;
  dataVector.reserve(totalSize);
  float *dataPtr = (float *) &dataVector[0];
 
  if(_nLvl == 1)
  {
    if (nc_var->get(dataPtr, _nTime, _nLat, _nLon) == 0) {
      cerr << "ERROR - " <<  methodName << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        return false;
    }
  }else{

    if (nc_var->get(dataPtr, _nTime, _nLvl, _nLat, _nLon) == 0) {
      cerr << "ERROR - " <<  methodName << endl;
        cerr << "  Cannot get data from input netcdf variable" << endl;
        return false;
    }
  }
  // unpack data with scale and offset if it is packed
  if(nc_var->get_att(_params->netcdf_vattr_scale) != 0)
  {
    double scale = nc_var->get_att(_params->netcdf_vattr_scale)->as_double(0);
    double offset = nc_var->get_att(_params->netcdf_vattr_offset)->as_double(0);
  
    for(size_t count = 0; count < totalSize; count++)
    {
      dataVector[count] = dataVector[count] * scale + offset;
    }
  }
  
  if(_isFlipped && _params->allowFlip)
  {
    unsigned int midLat = _nLat / 2;
    int indexA, indexB;
    float temp;
    for(unsigned int x=0; x<_nLvl; x++)
    {
      for(unsigned int y=0; y<midLat; y++)
      {
        for(unsigned int z=0; z<_nLon; z++)
	{
          indexA = ( x * _nLat * _nLon ) + ( y * _nLon ) + z;
          indexB = ( x * _nLat * _nLon ) + ( (_nLat - 1 - y) * _nLon ) + z;
          temp = dataVector[indexA];
          dataVector[indexA] = dataVector[indexB];
          dataVector[indexB] = temp;
        }
      }
    }    
  }
  
  // set up MdvxField headers
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
 
  if(_params->inputDimension == 3)
  {
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  }
  else {
    fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
    fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  }
  fhdr.dz_constant = false;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);

  fhdr.forecast_delta = _fhr * 3600;
  fhdr.forecast_time = dateTime + fhdr.forecast_delta;

  _proj.syncToFieldHdr(fhdr);
  

  // create MdvxField object, converting data to encoding and compression types

  MdvxField *field = new MdvxField(fhdr, _vhdr, dataPtr);

  // set names etc
  
  field->setFieldName(fieldName.c_str());
  field->setFieldNameLong(fieldNameLong.c_str());
  field->setUnits(units.c_str());
  field->setTransform("");
  
  *mdvx_field = field;

  return true;
}
