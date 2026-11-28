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
/*********************************************************************
 * ReadForest: An Abstract base class for various forest types. 
 *
 * Jason Craig
 *
 * RAP, NCAR, Boulder CO
 *
 * Dec 2009
 *
 *********************************************************************/

#include <iostream>
#include "InputField.hh"
#include "MdvReader.hh"

using namespace std;

InputField::InputField(Params *params, string name)
{
  this->_params = params;
  this->_header = NULL;
  this->_vHeader = NULL;
  this->_data = NULL;
  this->_isDummy = false;
  this->_name = name;
  this->_remap_type = Params::BASIC;
  this->_zIndexs = new float[_params->output_nz];
}

InputField::~InputField()
{
}

void InputField::clear()
{
  if(_header != NULL)
    delete _header;
  if(_vHeader != NULL)
    delete _vHeader;
  delete []this->_zIndexs;
}

void InputField::setDummy()
{
  _isDummy = true;
}

void InputField::setField(Mdvx::field_header_t *header, Mdvx::vlevel_header_t *vHeader, float *data)
{
  this->_data = data;
  this->_header = header;
  this->_vHeader = vHeader;

  if(_data != NULL && _header != NULL)
  {

    if(_header->proj_type != _params->projection_info.type ||
       _header->nx != _params->grid_info.nx ||
       _header->ny != _params->grid_info.ny || 
       (_header->nz != 1 && _header->nz != _params->output_nz))
    {
      
      switch( _params->projection_info.type) {
      case Params::PROJ_FLAT:
	_outproj.initFlat(_params->projection_info.origin_lat, 
			  _params->projection_info.origin_lon,
			  _params->projection_info.rotation);
	break;
      case Params::PROJ_LATLON:
	_outproj.initLatlon();
	break;
      case Params::PROJ_LAMBERT_CONF:
	_outproj.initLambertConf(_params->projection_info.origin_lat, 
				 _params->projection_info.origin_lon,
				 _params->projection_info.ref_lat_1,
				 _params->projection_info.ref_lat_2);
	break;
      }
      _outproj.setGrid(_params->grid_info.nx, _params->grid_info.ny, 
		       _params->grid_info.dx, _params->grid_info.dy,
		       _params->grid_info.minx, _params->grid_info.miny);
      
      _inproj.init(*_header);
    }
  }

}

void InputField::setMissing(float miss, float no_data)
{
  _missing_val = miss;
  _no_data_val = no_data;
}

float InputField::getVal(const int &i, const int &j, const int &k)
{

  if(_isDummy || _header == NULL || _vHeader == NULL || _data == NULL)
    return _no_data_val;

  float field_val;
  if(_header->proj_type != _params->projection_info.type ||
     _header->nx != _params->grid_info.nx ||
     _header->ny != _params->grid_info.ny || 
     (_header->nz != 1 && _header->nz != _params->output_nz))
  {    
    double lat, lon, x, y, z;
    _outproj.xyIndex2latlon(i, j, lat, lon);
    int ingrid = _inproj.latlon2xyIndex(lat, lon, x, y);
    z = _zIndexs[k];

    if(ingrid != -1 && x >= 0 && y >= 0)
      if(_remap_type == Params::BASIC)
	field_val = MdvReader::interp2(_header, (int)(x+.5), (int)(y+.5), (int)(z+.5), _data);
      else if(_remap_type == Params::INTERP_FLAT)
	field_val = MdvReader::interp2(_header, x, y, (int)(z+.5), _data);
      else if(_remap_type == Params::INTERP)
	field_val = MdvReader::interp3(_header, x, y, z, _data);
      else
	return _no_data_val;
    else
      field_val = _missing_val;

  } else 
  {
    if(_header->nz == 1)
      field_val = _data[(j*_header->nx)+i];
    else {
      field_val = _data[(k*_header->ny*_header->nx)+(j*_header->nx)+i];
    }
  }

  if(field_val == _header->missing_data_value)
    field_val = _missing_val;

  return field_val;
}
