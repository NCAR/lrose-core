// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// © University Corporation for Atmospheric Research (UCAR) 2009-2010. 
// All rights reserved.  The Government's right to use this data and/or 
// software (the "Work") is restricted, per the terms of Cooperative 
// Agreement (ATM (AGS)-0753581 10/1/08) between UCAR and the National 
// Science Foundation, to a "nonexclusive, nontransferable, irrevocable, 
// royalty-free license to exercise or have exercised for or on behalf of 
// the U.S. throughout the world all the exclusive rights provided by 
// copyrights.  Such license, however, does not include the right to sell 
// copies or phonorecords of the copyrighted works to the public."   The 
// Work is provided "AS IS" and without warranty of any kind.  UCAR 
// EXPRESSLY DISCLAIMS ALL OTHER WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
// ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE.  
//  
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <toolsa/copyright.h>

/**
 * @file DataHandlerMdv.cc
 */

#include "DataHandlerMdv.hh"
#include "Data.hh"
#include "Fields.hh"
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogMsg.hh>
#include <cmath>

//----------------------------------------------------------------
DataHandlerMdv::DataHandlerMdv(const Params &parms) : DataHandler(parms)
{
}

//----------------------------------------------------------------
DataHandlerMdv::~DataHandlerMdv()
{
}

//----------------------------------------------------------------
DataHandlerMdv::DataHandlerMdv(const DataHandlerMdv &d) : 
  DataHandler(d),
  _mdv(d._mdv),
  _field_hdr(d._field_hdr),
  _vlevel_hdr(d._vlevel_hdr)
{
  reset();
}

//----------------------------------------------------------------
DataHandler *DataHandlerMdv::clone(void) const
{
  DataHandlerMdv *r = new DataHandlerMdv(*this);
  return (DataHandler *)r;
}

//----------------------------------------------------------------
bool DataHandlerMdv::init(void)
{
  // read in data to get a template, any data will do.
  DsMdvxTimes T;

  DateTime dt(_parms._template_time[0], _parms._template_time[1],
	      _parms._template_time[2], _parms._template_time[3],
	      _parms._template_time[4], _parms._template_time[5]);
  time_t ttime = dt.utime();
  T.setArchive(_parms.template_url, ttime - 10000, ttime);
  time_t t;
  if (T.getLast(t))
  {
    printf("ERROR no data in range for template url\n");
    return false;
  }
  printf("Most recent data time in template url = %s\n",
	 DateTime::strn(t).c_str());


  MdvxField *f=NULL;

  f = _read(t);
  if (f == NULL)
  {
    return false;
  }

  if (_parms.change_grid)
  {
    f = _regrid();
    if (f == NULL)
    {
      return false;
    }
  }


  // make a copy of field to delete later, prior to clearing all fields
  MdvxField *fcopy = new MdvxField(*f);

  _mdv.clearFields();

  // create and store a field for each thing that is configured for output
  for (int i=0; i<_parms.fields_n; ++i)
  {
    _addField(*fcopy, _parms._fields[i]);
  }
  delete fcopy;

  _reset();
  return true;
}

//----------------------------------------------------------------
void DataHandlerMdv::reset(void)
{
  for (int i=0; i<_parms.fields_n; ++i)
  {
    MdvxField *f = _mdv.getFieldByNum(i);
    fl32 *data = (fl32 *)f->getVol();
    Mdvx::field_header_t fh = f->getFieldHeader();
    _data[(int)_parms._fields[i]] = DataGrid(fh.nx, fh.ny, fh.nz,
					     fh.missing_data_value,
					     data);
  }      
  _reset();
}

//----------------------------------------------------------------
void DataHandlerMdv::store(const Data &data)
{
  map<int,DataGrid>::iterator ii;
  for (ii=_data.begin(); ii!=_data.end(); ++ii)
  {
    Params::Field_t f = (Params::Field_t)(ii->first);
    bool motion = (f == Params::VX || f == Params::VY || 
		   f == Params::VZ || f == Params::RADIAL_VEL);
    double v;
    if (f == Params::RADIAL_VEL)
    {
      v = _radialVel(data);
    }
    else
    {
      v = data.value(f);
    }
    _data[f].setValue(_ix, _iy, _iz, v, motion);
  }
}

//----------------------------------------------------------------
void DataHandlerMdv::write(const time_t &currentTime)
{
  // write out the results for this time
  Mdvx::master_header_t mh = _mdv.getMasterHeader();
  mh.time_gen = time(0);
  mh.time_begin = currentTime;
  mh.time_end = currentTime;
  mh.time_centroid = currentTime;
  // half the step size
  mh.time_expire = currentTime +
    static_cast<int>(_parms.simulation_step_minutes*30);
  _mdv.setMasterHeader(mh);
  for (int i=0; i<_mdv.getNFields(); ++i)
  {
    MdvxField *f = _mdv.getFieldByNum(i);
    f->convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_GZIP);
  }
  _mdv.setWriteLdataInfo();
  if (_mdv.writeToDir(_parms.output_url))
  {
    printf("ERROR writing to %s\n", _parms.output_url);
  }
  for (int i=0; i<_mdv.getNFields(); ++i)
  {
    MdvxField *f = _mdv.getFieldByNum(i);
    f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  }
}

//----------------------------------------------------------------
void DataHandlerMdv::_initIndexing(void)
{
  _ix = _iy = _iz = 0;
}

//----------------------------------------------------------------
Xyz DataHandlerMdv::_computeXyzMeters(void)
{
  if (_polar)
  {
    _cx = (_field_hdr.grid_minx + _ix*_field_hdr.grid_dx)*1000.0;
    // radar convention assumed:
    _cy = 90.0 - (_field_hdr.grid_miny + _iy*_field_hdr.grid_dy);
    while (_cy < 0)
    {
      _cy += 360;
    }
    while (_cy > 360.0)
    {
      _cy -= 360;
    }
    _cy *= 3.14159/180.0;
    _cz = (_vlevel_hdr.level[_iz])*3.14159/180.0;

    return Xyz(_cx*cos(_cy)*cos(_cz), _cx*sin(_cy)*cos(_cz), _cx*sin(_cz));
  }
  else
  {
    _cx = (_field_hdr.grid_minx + _ix*_field_hdr.grid_dx)*1000.0;
    _cy = (_field_hdr.grid_miny + _iy*_field_hdr.grid_dy)*1000.0;
    _cz = (_vlevel_hdr.level[_iz])*1000.0;
    return Xyz(_cx, _cy, _cz);
  }
}

//----------------------------------------------------------------
bool DataHandlerMdv::_increment(void)
{
  if (++_ix >= _field_hdr.nx)
  {
    _ix = 0;
    if (++_iy >= _field_hdr.ny)
    {
      _iy = 0;
      if (++_iz >= _field_hdr.nz)
      {
	return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------
MdvxField *DataHandlerMdv::_read(const time_t &t)
{
  _mdv.setReadTime(Mdvx::READ_FIRST_BEFORE, _parms.template_url, 0, t);
  _mdv.clearReadFields();
  _mdv.addReadField(_parms.template_url_field);
  if (_parms.change_grid)
  {
    _mdv.setReadVlevelLimits(_parms._vlevel_range[0],
			     _parms._vlevel_range[1]);
  }                             
  if (_mdv.readVolume())
  {
    printf("ERROR reading %s field, template URL\n", _parms.template_url_field);
    return NULL;
  }

  // pull out our field and header info to figure out how to map from
  // gridpoint to point in space
  MdvxField *f = _mdv.getFieldByName(_parms.template_url_field);
  if (f == NULL)
  {
    LOGF(LogMsg::ERROR, "reading field %s", _parms.template_url_field);
    return NULL;
  }

  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  _field_hdr = f->getFieldHeader();
  _vlevel_hdr = f->getVlevelHeader();
  if (!_setProjectionType(_field_hdr))
  {
    return NULL;
  }

  return f;
}

//----------------------------------------------------------------
MdvxField *DataHandlerMdv::_regrid(void)
{
  // shrink needed
  Mdvx::master_header_t master_hdr = _mdv.getMasterHeader();

  // set the projection using input data and params
  MdvxProj proj(master_hdr, _field_hdr);
  Mdvx::coord_t c = proj.getCoord();
  // change y (azimuth)
  int ny = (_parms._azimuth_range[1] - _parms._azimuth_range[0] + c.dy)/c.dy;
  proj.setGrid(c.nx, ny, c.dx, c.dy, c.minx, _parms._azimuth_range[0]);

  Mdvx::master_header_t m = master_hdr;
  m.max_nx = 0;
  m.max_ny = 0;
  m.max_nz = 0;

  Mdvx::field_header_t fieldHdr = _field_hdr;

  // Fill in the projection information in the field header.
  proj.syncXyToFieldHdr(fieldHdr);
  
  MdvxField *f = new MdvxField(fieldHdr, _vlevel_hdr, (void *)0, true,
			       false);
  // copy the data in now. (not needed, actually)
  fl32 *fo = (fl32 *)f->getVol();
  for (int i=0; i<c.nx*ny*c.nz; ++i)
  {
    fo[i] = 0;
  }

  _mdv = DsMdvx();
  _mdv.setMasterHeader(m);
  _mdv.setDataCollectionType(Mdvx::DATA_MEASURED);
  _mdv.addField(f);
  _field_hdr = fieldHdr;

  // read the field back out (make a copy)
  f = _mdv.getFieldByName(_parms.template_url_field);
  if (f == NULL)
  {
    LOGF(LogMsg::ERROR, "reading field %s", _parms.template_url_field);
    return NULL;
  }
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  return f;
}

//----------------------------------------------------------------
bool DataHandlerMdv::_setProjectionType(const Mdvx::field_header_t &f)
{
  _polar = false;
  switch (f.proj_type)
  {
  case Mdvx::PROJ_FLAT:
    LOG(LogMsg::DEBUG, "FLAT data");
    break;
  case Mdvx::PROJ_POLAR_RADAR:
    LOG(LogMsg::DEBUG, "POLAR RADAR data");
    _polar = true;
    break;
  default:
    LOGF(LogMsg::ERROR, "Projection %d not supported", f.proj_type);
    return false;
  }

  switch (f.vlevel_type)
  {
  case Mdvx::VERT_TYPE_Z:
    if (_polar)
    {
      LOG(LogMsg::ERROR,
	  "Polar radar projection with vert type z not supported");
      return false;
    }
    else
    {
      LOG(LogMsg::DEBUG, "Vertical type = z");
    }
    break;
  case Mdvx::VERT_TYPE_ELEV:
  case Mdvx::VERT_VARIABLE_ELEV:
    if (_polar)
    {
      LOG(LogMsg::DEBUG, "ELEVATION ANGLES\n");
    }
    else
    {
      LOG(LogMsg::ERROR, "Flat projection with elevations not supported");
      return false;
    }
    break;
  default:
    LOGF(LogMsg::ERROR, "Vertical type %d not supported", f.vlevel_type);
    return false;
  }
  return true;
}

//----------------------------------------------------------------
void DataHandlerMdv::_addField(const MdvxField &f, const Params::Field_t t)
{    
  MdvxField *field = new MdvxField(f);
    
  // here want to change missing data values, names, and units
  string name = Fields::fieldName(t);
  string units = Fields::fieldUnits(t);
  MdvxField *fnew = new MdvxField(*field);
  Mdvx::field_header_t fh = fnew->getFieldHeader();
  fh.bad_data_value = Fields::fieldMissingValue(t);
  fh.missing_data_value = Fields::fieldMissingValue(t);
  fnew->setFieldHeader(fh);
  fnew->setFieldName(name.c_str());
  fnew->setFieldNameLong(name.c_str());
  fnew->setUnits(units.c_str());
  fnew->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  fl32 *data = (fl32 *)fnew->getVol();
  for (int j=0; j<fh.nx*fh.ny*fh.nz; ++j)
  {
    data[j] = fh.missing_data_value;
  }
  _mdv.addField(fnew);
  _data[(int)t] = DataGrid(fh.nx, fh.ny, fh.nz, fh.missing_data_value,
			   data);
}


