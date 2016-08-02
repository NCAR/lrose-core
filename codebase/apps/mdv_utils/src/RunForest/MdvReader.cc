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
/*
 *   Module: MdvReader.cc
 *
 *   Author: Jason Craig
 *
 */

#include <math.h>

#include "MdvReader.hh"

float LAT_TO_KM  = 111.0;
float LONG_TO_KM = 111.321;
float FT_TO_KM = 0.0003048;

MdvReader::MdvReader(Params *params, Params::input_t mdvInfo)
{
  _params = params;
  _url = mdvInfo.mdv_url;
  _mdvx = new DsMdvx();
  _mdvx->setDebug(_params->debug);
  _master_header.time_centroid = 0;
  _loadMargin = mdvInfo.mdv_read_margin;
  _loadOffset = mdvInfo.mdv_read_offset;
  _vert_min = mdvInfo.vert_min;
  _vert_max = mdvInfo.vert_max;
  _do_comp = mdvInfo.do_composite;
  _path_in_use = "unknown";
}

MdvReader::~MdvReader()
{
  delete _mdvx;
}

int MdvReader::setTime(time_t loadTime)
{
  _path_in_use = "unknown";

  _mdvx->clearFields();
  _mdvx->clearChunks();

  loadTime += _loadOffset * 60;

  if(_vert_min > -1 && _vert_max > -1)
    _mdvx->setReadVlevelLimits( _vert_min, _vert_max);

  if(_do_comp)
    _mdvx->setReadComposite();

  _mdvx->setReadTime(Mdvx::READ_CLOSEST, _url, _loadMargin*60, loadTime);
  if(_mdvx->readAllHeaders() == -1) {
    _master_header.time_centroid = 0;
    return NO_MDV_AT_TIME;
  }

  _master_header = _mdvx->getMasterHeaderFile();

  if(loadTime < _master_header.time_centroid + (_loadMargin*60) && 
     loadTime > _master_header.time_centroid - (_loadMargin*60)) {

    _path_in_use = _mdvx->getPathInUse();

    cout << "Loading Mdv file: " << _path_in_use << endl;

    if (_mdvx->readVolume()) {
      cerr << "ERROR Cannot read in mdv file: " << _path_in_use << endl;
      cerr << _mdvx->getErrStr() << endl;
      _master_header.time_centroid = 0;
      return READ_FAILURE;
    }

    _fileTime = _master_header.time_centroid;

    return SUCCESS;
  } else {
    _master_header.time_centroid = 0;
    return NO_MDV_AT_TIME;
  }
}

time_t MdvReader::getLoadedTime()
{
  if(_master_header.time_centroid == 0)
    return 0;
  return _master_header.time_centroid;
}

int MdvReader::getForecastDelta()
{
  if(_master_header.time_centroid == 0)
    return 0;
  return _master_header.time_centroid - _master_header.time_begin;
}

string MdvReader::getPathInUse()
{
  if(_master_header.time_centroid == 0)
  {
    return string(_url) + " is not available";
  }

  // return _mdvx->getPathInUse();
  return _path_in_use;
}

void MdvReader::clearRead()
{
  _mdvx->clearFields();
  _mdvx->clearChunks();

}

bool MdvReader::getField(char *fieldName, InputField *ifield, Params::remap_option_t remap_type, float vert_level, 
			 Mdvx::vlevel_header_t *outVHeader)
{
  //if(_master_header.grid_orientation != Mdvx::ORIENT_SN_WE || _master_header.data_ordering != Mdvx::ORDER_XYZ) {
  //cerr << "File " << _mdvx->getPathInUse() << " has unknown grid or data ordering." << endl;
  //return false;
  //}
  if(_master_header.time_centroid == 0)
    return false;

  MdvxField *field = _mdvx->getField(fieldName);
  if(field == NULL) {
    cerr << "Can't find field " << fieldName << " in input file " << _mdvx->getPathInUse() << endl;
    return false;
  }
  
  return _getField(field, ifield, remap_type, vert_level, outVHeader);
}

bool MdvReader::getField(int fieldNumber, InputField *ifield, Params::remap_option_t remap_type, float vert_level, 
			 Mdvx::vlevel_header_t *outVHeader)
{
  if(_master_header.time_centroid == 0)
    return false;

  MdvxField *field = _mdvx->getField(fieldNumber);
  if(field == NULL) {
    cerr << "Can't find field number " << fieldNumber << " in input file " << _mdvx->getPathInUse() << endl;
    return false;
  }

  return _getField(field, ifield, remap_type, vert_level, outVHeader);
}

bool MdvReader::_getField(MdvxField *field, InputField *ifield, Params::remap_option_t remap_type, float vert_level, 
			  Mdvx::vlevel_header_t *outVHeader)
{
  field->convertType( Mdvx::ENCODING_FLOAT32 );

  const Mdvx::field_header_t fhdr = field->getFieldHeader();

  int nz = _params->output_nz;

  if(fhdr.nz != 1 && nz != 1 && fhdr.nz != _params->output_nz && outVHeader == NULL) {
    cout << "ERROR: 3D field, " << fhdr.field_name 
	 << ", does not match output levels, and is before any field that does match output levels, cannot remap." << endl;
    return false;
  }

  if(fhdr.nz == 1) {
    for(int z = 0; z < nz; z++)
      ifield->setZIndex(z, 0);
  } else {
    if(nz != fhdr.nz) {
      if(nz == 1)
	ifield->setZIndex(0, vert_level);
      else
	for(int z = 0; z < nz; z++)
	  ifield->setZIndex(z, _getZIndex(outVHeader->level[z], outVHeader->type[0], fhdr, field->getVlevelHeader()) );
    } else {
      for(int z = 0; z < nz; z++)
	ifield->setZIndex(z, z);
    }
  }

  if(fhdr.proj_type != _params->projection_info.type ||
     fhdr.nx != _params->grid_info.nx ||
     fhdr.ny != _params->grid_info.ny || 
     (fhdr.nz != 1 && fhdr.nz != _params->output_nz))
  {
    if(remap_type == Params::MAX || remap_type == Params::MIN ||
       remap_type == Params::MEAN) {
      bool create_mean = false, create_min =  false;
      if(remap_type != Params::MEAN)
	create_mean = true;
      if(remap_type != Params::MIN)
	create_min = true;
      _remapMeanMaxMin(field, remap_type, ifield->getzIndexs(), outVHeader->type[0]);
    } else {
      if(_params->output_input_fields) {
	_remapBasic(field, remap_type, ifield->getzIndexs(), outVHeader->type[0]);
      }
    }
  }

  ifield->setField(new Mdvx::field_header_t(field->getFieldHeader()), new Mdvx::vlevel_header_t(field->getVlevelHeader()),
		   (float *)field->getVol());

  return true;
}


float MdvReader::_getZIndex(float altitude, int out_type, Mdvx::field_header_t fieldHdr, Mdvx::vlevel_header_t lvlHdr)
{
  int in_type = fieldHdr.vlevel_type;
  if(in_type != lvlHdr.type[0])
    in_type = lvlHdr.type[0];
  if(in_type == Mdvx::VERT_TYPE_UNKNOWN)
    in_type = lvlHdr.type[0];

  if(in_type == Mdvx::VERT_TYPE_SURFACE && fieldHdr.nz == 2) { 
    return 0;
  }

  if(out_type != Mdvx::VERT_TYPE_ZAGL_FT) {
    if(out_type == Mdvx::VERT_TYPE_Z) {
      altitude /= FT_TO_KM;
    } else if(out_type == Mdvx::VERT_FLIGHT_LEVEL) {
      altitude *= 100.0;
    } else {
      cerr << "ERROR: Cannot calculate zIndex from altitude due to unknown Mdv vertical type for output." << endl;
      return -1.0;
    }
  }

  if(in_type == Mdvx::VERT_TYPE_Z) {
    altitude *= FT_TO_KM;
  } else if(in_type == Mdvx::VERT_FLIGHT_LEVEL) {
    altitude /= 100.0;
  } else if(in_type == Mdvx::VERT_TYPE_ZAGL_FT) {
    altitude = altitude;
  } else {
    cerr << "ERROR: Cannot calculate zIndex from altitude due to unknown Mdv vertical type for field " <<
	 fieldHdr.field_name << endl;
    return -1.0;
  }

  int z = 0;
  while(z < fieldHdr.nz && lvlHdr.level[z] < altitude)
    z++;
  z--;

  float zInd = 0.0;
  if(z != -1 && z != fieldHdr.nz -1) {
    float zdiff = (lvlHdr.level[z] - altitude) + (altitude - lvlHdr.level[z+1]);
    zInd = (lvlHdr.level[z] - altitude) / zdiff;
  } else {
    if(z == -1)
      return 0.0;
    if(z == fieldHdr.nz -1)
      return fieldHdr.nz -1;
  }

  return z+zInd;
}


float MdvReader::interp3(Mdvx::field_header_t *fieldHdr, double x, double y, double z, float *field)
{
  if(field == NULL || x < 0 || y < 0 || z == -1.0 || (int)x+1 >= fieldHdr->nx || (int)y+1 >= fieldHdr->ny)
    return fieldHdr->missing_data_value;
  if(z < 1.0)
    return interp2(fieldHdr, x, y, 0, field);
  if(z > fieldHdr->nz)
    return interp2(fieldHdr, x, y, fieldHdr->nz-1, field);

  float l2dInterp, u2dInterp;
  l2dInterp = interp2(fieldHdr, x, y, (int)z, field);

  u2dInterp = interp2(fieldHdr, x, y, (int)z+1, field);

  float val = (l2dInterp * (1-(z-(int)z))) + (u2dInterp * (z-(int)z));
  if(!finite(val))// || val > 1e100 || val < -1e100)
    return fieldHdr->missing_data_value;
  else
    return val;
}


float MdvReader::interp2(Mdvx::field_header_t *fieldHdr, double x, double y, float *field)
{
  if(field == NULL || x < 0 || y < 0 || (int)x+1 >= fieldHdr->nx || (int)y+1 >= fieldHdr->ny)
    return fieldHdr->missing_data_value;
  if(field[((int)y*fieldHdr->nx)+(int)x] == fieldHdr->missing_data_value ||
     field[((int)y*fieldHdr->nx)+(int)x+1] == fieldHdr->missing_data_value ||
     field[(((int)y+1)*fieldHdr->nx)+(int)x] == fieldHdr->missing_data_value ||
     field[(((int)y+1)*fieldHdr->nx)+(int)x+1] == fieldHdr->missing_data_value) {
    if(field[((int)(y+.5)*fieldHdr->nx)+(int)(x+.5)] == fieldHdr->missing_data_value)
      return fieldHdr->missing_data_value;
    else
      return field[((int)(y+.5)*fieldHdr->nx)+(int)(x+.5)];
  }
  float val = (field[((int)y*fieldHdr->nx)+(int)x] * (1-(x-(int)x)) + 
	       field[((int)y*fieldHdr->nx)+(int)x+1] * (x-(int)x)) * (1-(y-(int)y)) + 
    (field[(((int)y+1)*fieldHdr->nx)+(int)x] * (1-(x-(int)x)) + 
     field[(((int)y+1)*fieldHdr->nx)+(int)x+1] * (x-(int)x)) * (y-(int)y);
  if(!finite(val))// || val > 1e100 || val < -1e100)
    return fieldHdr->missing_data_value;
  else
    return val;
}

float MdvReader::interp2(Mdvx::field_header_t *fieldHdr, double x, double y, int z, float *field)
{
  if(field == NULL || x < 0 || y < 0 || (int)x+1 >= fieldHdr->nx || (int)y+1 >= fieldHdr->ny)
    return fieldHdr->missing_data_value;
  if(z < 0)
    z = 0;
  if(z >= fieldHdr->nz)
    z = fieldHdr->nz -1;
  if(field[(z*fieldHdr->ny*fieldHdr->nx)+((int)y*fieldHdr->nx)+(int)x] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+((int)y*fieldHdr->nx)+(int)x+1] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+(((int)y+1)*fieldHdr->nx)+(int)x] == fieldHdr->missing_data_value ||
     field[(z*fieldHdr->ny*fieldHdr->nx)+(((int)y+1)*fieldHdr->nx)+(int)x+1] == fieldHdr->missing_data_value) {

    if(field[(z*fieldHdr->ny*fieldHdr->nx)+((int)(y+.5)*fieldHdr->nx)+(int)(x+.5)] == fieldHdr->missing_data_value)
      return fieldHdr->missing_data_value;
    else
      return field[(z*fieldHdr->ny*fieldHdr->nx)+((int)(y+.5)*fieldHdr->nx)+(int)(x+.5)];
  }

  float val = (field[(z*fieldHdr->ny*fieldHdr->nx)+((int)y*fieldHdr->nx)+(int)x] * (1-(x-(int)x)) + 
	       field[(z*fieldHdr->ny*fieldHdr->nx)+((int)y*fieldHdr->nx)+(int)x+1] * (x-(int)x)) * (1-(y-(int)y)) + 
    (field[(z*fieldHdr->ny*fieldHdr->nx)+(((int)y+1)*fieldHdr->nx)+(int)x] * (1-(x-(int)x)) + 
     field[(z*fieldHdr->ny*fieldHdr->nx)+(((int)y+1)*fieldHdr->nx)+(int)x+1] * (x-(int)x)) * (y-(int)y);

  if(!finite(val))// || val > 1e100 || val < -1e100)
    return fieldHdr->missing_data_value;
  else
    return val;
}


void MdvReader::_remapBasic(MdvxField* inputField, Params::remap_option_t remap_type, float *zIndexs, int zType)
{
  Mdvx::field_header_t fhdr = inputField->getFieldHeader();
  
  int nx = _params->grid_info.nx;
  int ny = _params->grid_info.ny;
  int nz = _params->output_nz;
  int inx = fhdr.nx;
  int iny = fhdr.ny;
  int inz = fhdr.nz;

  MdvxProj inproj, outproj;
  switch( _params->projection_info.type) {
    case Params::PROJ_FLAT:
      outproj.initFlat(_params->projection_info.origin_lat, 
		       _params->projection_info.origin_lon,
		       _params->projection_info.rotation);
      break;
    case Params::PROJ_LATLON:
      outproj.initLatlon();
      break;
    case Params::PROJ_LAMBERT_CONF:
      outproj.initLambertConf(_params->projection_info.origin_lat, 
			      _params->projection_info.origin_lon,
			      _params->projection_info.ref_lat_1,
			      _params->projection_info.ref_lat_2);
      break;
  }
  outproj.setGrid(nx, ny, _params->grid_info.dx, _params->grid_info.dy,
		  _params->grid_info.minx, _params->grid_info.miny);
  
  inproj.init(fhdr);

  // Keep 2d fields 2d
  if(inz == 1) {
    nz = 1;
  }

  float *odata = new float[nx*ny*nz];
  float *idata = (float *)inputField->getVol();

  double lat, lon;
  double ix, iy;
  for(int y = 0; y < ny; y++) 
  {
    for(int x = 0; x < nx; x++)
    {
      outproj.xyIndex2latlon(x, y, lat, lon);

      int ingrid = inproj.latlon2xyIndex(lat, lon, ix, iy);

      if(ingrid != -1 && ix >= 0 && iy >= 0)
	for(int z = 0; z < nz; z++)
	  if(remap_type == Params::BASIC)
	    odata[(z*ny*nx)+(y*nx)+x] = interp2(&fhdr, (int)(ix+.5), (int)(iy+.5), (int)(zIndexs[z]+.5), idata);
	  else if(remap_type == Params::INTERP_FLAT)
	    odata[(z*ny*nx)+(y*nx)+x] = interp2(&fhdr, ix, iy, (int)(zIndexs[z]+.5), idata);
	  else
	    odata[(z*ny*nx)+(y*nx)+x] = interp3(&fhdr, ix, iy, zIndexs[z], idata);	    
      else
	for(int z = 0; z < nz; z++)
	  odata[(z*ny*nx)+(y*nx)+x] = fhdr.missing_data_value;
    }
  }

  fhdr.nx = _params->grid_info.nx;
  fhdr.ny = _params->grid_info.ny;
  fhdr.grid_minx = _params->grid_info.minx;
  fhdr.grid_miny = _params->grid_info.miny;
  fhdr.grid_dx = _params->grid_info.dx;
  fhdr.grid_dy = _params->grid_info.dy;
  fhdr.proj_type = _params->projection_info.type;
  fhdr.proj_origin_lat = _params->projection_info.origin_lat;
  fhdr.proj_origin_lon = _params->projection_info.origin_lon;
  fhdr.proj_param[0] = _params->projection_info.ref_lat_1;
  fhdr.proj_param[1] = _params->projection_info.ref_lat_2;
  fhdr.volume_size =
    fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes; 

  if(inz != 1 && nz != inz) {
      fhdr.nz = nz;
      fhdr.dz_constant = 0;
      fhdr.grid_dz = 0.0;
      fhdr.grid_minz = zIndexs[0];
      fhdr.vlevel_type = zType;
      fhdr.volume_size =
	fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;

      Mdvx::vlevel_header_t vHdr = inputField->getVlevelHeader();
      for(int z; z < nz; z++) {
	vHdr.type[z] = zType;
	vHdr.level[z] = zIndexs[z];
      }
      inputField->setVlevelHeader(vHdr);
  }

  inputField->setFieldHeader(fhdr);
  inputField->setVolData(odata, fhdr.volume_size, Mdvx::ENCODING_FLOAT32);

  delete []odata;

}

/*********************************************************************
 * _remapMeanMaxMin() - Remap the input field using mean, max or min function.
 */

void MdvReader::_remapMeanMaxMin(MdvxField* inputField, Params::remap_option_t remap_type, float *zIndexs, int zType)
{
  Mdvx::field_header_t fhdr = inputField->getFieldHeader();
  
  int nx = _params->grid_info.nx;
  int ny = _params->grid_info.ny;
  int nz = _params->output_nz;

  int inx = fhdr.nx;
  int iny = fhdr.ny;
  int inz = fhdr.nz;


  MdvxProj inproj, outproj;
  switch( _params->projection_info.type) {
    case Params::PROJ_FLAT:
      outproj.initFlat(_params->projection_info.origin_lat, 
		       _params->projection_info.origin_lon,
		       _params->projection_info.rotation);
      break;
    case Params::PROJ_LATLON:
      outproj.initLatlon();
      break;
    case Params::PROJ_LAMBERT_CONF:
      outproj.initLambertConf(_params->projection_info.origin_lat, 
			      _params->projection_info.origin_lon,
			      _params->projection_info.ref_lat_1,
			      _params->projection_info.ref_lat_2);
      break;
  }
  outproj.setGrid(nx, ny, _params->grid_info.dx, _params->grid_info.dy,
		  _params->grid_info.minx, _params->grid_info.miny);

  inproj.init(fhdr);

  // Keep 2d fields 2d
  if(inz == 1) {
    nz = 1;
  }

  float *odata = new float[nx*ny*nz];
  float *idata = (float *)inputField->getVol();

  int *mcount = NULL;
  if(remap_type == Params::MEAN) {
    mcount = new int[nx*ny*nz];
  }

  for(int y = 0; y < ny; y++) 
    for(int x = 0; x < nx; x++)
      for(int z = 0; z < nz; z++) {
	odata[(z*ny*nx)+(y*nx)+x] = fhdr.missing_data_value;
	if(remap_type == Params::MEAN) {
	  mcount[(z*ny*nx)+(y*nx)+x] = 0;
	}
      }

  double lat, lon;
  double ox, oy;
  for(int iy = 0; iy < iny; iy++) 
  {
    for(int ix = 0; ix < inx; ix++)
    {

      inproj.xyIndex2latlon(ix, iy, lat, lon);

      int ingrid = outproj.latlon2xyIndex(lat, lon, ox, oy);
      int fox = (int)(ox+.5);
      int foy = (int)(oy+.5);

      if(ingrid != -1 && ox >= 0 && oy >= 0) {
	for(int oz = 0; oz < nz; oz++) {

	  int iz = (int)(zIndexs[oz]+.5);

	  if(remap_type == Params::MEAN) {
	    if(idata[(iz*iny*inx)+(iy*inx)+ix] != fhdr.missing_data_value) {
	      mcount[(oz*ny*nx)+(foy*nx)+fox] ++;
	      if(odata[(oz*ny*nx)+(foy*nx)+fox] == fhdr.missing_data_value)
		odata[(oz*ny*nx)+(foy*nx)+fox] = idata[(iz*iny*inx)+(iy*inx)+ix];
	      else
		odata[(oz*ny*nx)+(foy*nx)+fox] += idata[(iz*iny*inx)+(iy*inx)+ix];
	    }
	  } else if(remap_type == Params::MIN) {
	    if(idata[(iz*iny*inx)+(iy*inx)+ix] != fhdr.missing_data_value &&
	       (odata[(oz*ny*nx)+(foy*nx)+fox] == fhdr.missing_data_value ||
		odata[(oz*ny*nx)+(foy*nx)+fox] > idata[(iz*iny*inx)+(iy*inx)+ix]))
	      odata[(oz*ny*nx)+(foy*nx)+fox] = idata[(iz*iny*inx)+(iy*inx)+ix];
	  } else {
	    if(idata[(iz*iny*inx)+(iy*inx)+ix] != fhdr.missing_data_value &&
	       (odata[(oz*ny*nx)+(foy*nx)+fox] == fhdr.missing_data_value ||
		odata[(oz*ny*nx)+(foy*nx)+fox] < idata[(iz*iny*inx)+(iy*inx)+ix]))
	      odata[(oz*ny*nx)+(foy*nx)+fox] = idata[(iz*iny*inx)+(iy*inx)+ix];
	  }

	}
      }

    }
  }

  if(remap_type == Params::MEAN)
    for(int y = 0; y < ny; y++) 
      for(int x = 0; x < nx; x++)
	for(int z = 0; z < nz; z++)
	  if(mcount[(z*ny*nx)+(y*nx)+x] > 1)
	    odata[(z*ny*nx)+(y*nx)+x] /= mcount[(z*ny*nx)+(y*nx)+x];


  fhdr.nx = _params->grid_info.nx;
  fhdr.ny = _params->grid_info.ny;
  fhdr.grid_minx = _params->grid_info.minx;
  fhdr.grid_miny = _params->grid_info.miny;
  fhdr.grid_dx = _params->grid_info.dx;
  fhdr.grid_dy = _params->grid_info.dy;
  fhdr.proj_type = _params->projection_info.type;
  fhdr.proj_origin_lat = _params->projection_info.origin_lat;
  fhdr.proj_origin_lon = _params->projection_info.origin_lon;
  fhdr.proj_param[0] = _params->projection_info.ref_lat_1;
  fhdr.proj_param[1] = _params->projection_info.ref_lat_2;
  fhdr.volume_size =
    fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes; 

  if(inz != 1 && nz != inz) {
      fhdr.nz = nz;
      fhdr.dz_constant = 0;
      fhdr.grid_dz = 0.0;
      fhdr.grid_minz = zIndexs[0];
      fhdr.vlevel_type = zType;
      fhdr.volume_size =
	fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;

      Mdvx::vlevel_header_t vHdr = inputField->getVlevelHeader();
      for(int z; z < nz; z++) {
	vHdr.type[z] = zType;
	vHdr.level[z] = zIndexs[z];
      }
      inputField->setVlevelHeader(vHdr);
  }

  inputField->setFieldHeader(fhdr);
  inputField->setVolData(odata, fhdr.volume_size, Mdvx::ENCODING_FLOAT32);

  delete []odata;
  if(remap_type == Params::MEAN) {
    delete []mcount;
  }
}
