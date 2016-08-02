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
/////////////////////////////////////////////////////////////
// OutputField.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2002
//
///////////////////////////////////////////////////////////////

#include <cmath>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <physics/IcaoStdAtmos.hh>
#include "OutputField.hh"
#include "InputField.hh"
#include "GribMgr.hh"

using namespace std;

// constructor

OutputField::OutputField(const string &prog_name,
			 const Params  &params,
			 const string &name,
			 const string &long_name,
			 const string &units,
			 Params::grib_field_id_t grib_field_id,
			 Params::units_conversion_t units_conversion,
			 Params::encoding_type_t encoding) :
  _progName(prog_name),
  _params(params),
  _name(name),
  _longName(long_name),
  _units(units),
  _gribFieldId(grib_field_id),
  _unitsConversion(units_conversion),
  _encoding(encoding)

{

  _nX = 0;
  _nY = 0;
  _dX = 0;
  _dY = 0;
  _minX = 0;
  _minY = 0;
  _maxX = 0;
  _maxY = 0;
  _data = NULL;
  
}

// destructor

OutputField::~OutputField()

{
  clear();
}

////////////////////////
// add a grib field tile
//
// Only adds the pointer.
// This object does not own the memory associated with this pointer.

void OutputField::addInputField(InputField *tile)

{
  _fields.push_back(tile);
}

////////////////////////////
// assemble field from tiles

void OutputField::assemble()

{

  // this does not apply to WSPD

  if (_gribFieldId == Params::WSPD) {
    return;
  }

  // compute dx and dy, take max

  _dX = 0.0;
  _dY = 0.0;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Pjg *proj = _fields[ii]->getProjection();
    _dX = MAX(proj->getDx(), _dX);
    _dY = MAX(proj->getDy(), _dY);
  }
  
  // compute min and max X and Y values
  
  _minX = 360;
  _minY = 90;
  _maxX = -360;
  _maxY = -90;
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    Pjg *proj = _fields[ii]->getProjection();
    _minX = MIN(_minX, proj->getMinx());
    _minY = MIN(_minY, proj->getMiny());
    int nX = proj->getNx();
    int nY = proj->getNy();
    _maxX = MAX(_maxX, proj->getMinx() + (nX - 1) * _dX);
    _maxY = MAX(_maxY, proj->getMiny() + (nY - 1) * _dY);
  }

  _nX = (int) ((_maxX - _minX) / _dX + 0.5) + 1;
  _nY = (int) ((_maxY - _minY) / _dY + 0.5) + 1;

  // set up vertical levels array

  _levels.clear();
  const vector<int> &levels0(_fields[0]->getLevels());
  for (size_t ii = 0; ii < levels0.size(); ii++) {
    int level = levels0[ii];
    bool allPresent = true;
    for (size_t jj = 0; jj < _fields.size(); jj++) {
      if (_fields[jj]->findLevelIndex(level) < 0) {
	allPresent = false;
	break;
      } // if
    } // jj
    if (allPresent) {
      if (_levels.size() < MDV_MAX_VLEVELS) {
	_levels.push_back(level);
      }
    }
  }

  if (_params.debug) {
    print(cerr);
  }

  // load up data array

  int nxy = _nX * _nY;
  int npoints = nxy * _levels.size();
  if (_data) {
    delete[] _data;
  }
  _data = new fl32[npoints];
  fl32 missing = GribMgr::getMissingVal();
  for (int ii = 0; ii < npoints; ii++) {
    _data[ii] = missing;
  }

  for (size_t ilevel = 0; ilevel < _levels.size(); ilevel++) {

    fl32 *outPlane = _data + (ilevel * nxy);

    for (size_t itile = 0; itile < _fields.size(); itile++) {
      
      const Pjg *proj = _fields[itile]->getProjection();
      double dx = proj->getDx();
      double dy = proj->getDy();
      int nx = proj->getNx();
      int ny = proj->getNy();
      const fl32 *inPlane = _fields[itile]->getData()[ilevel];
      
      double yy = proj->getMiny();
      for (int iy = 0; iy < ny; iy++, yy += dy) {
	double xx = proj->getMinx();
	for (int ix = 0; ix < nx; ix++,  xx += dx, inPlane++) {
	  fl32 value = *inPlane;
	  int ixOut = (int) ((xx - _minX) / _dX + 0.5);
	  int iyOut = (int) ((yy - _minY) / _dY + 0.5);
	  outPlane[iyOut * _nX + ixOut] = value;
	} // ix
      } // iy

    } // itile
    
  } // ilevel

  // convert as appropriate

  switch (_unitsConversion) {
    
  case Params::MPS_TO_KNOTS:
    {
      fl32 *dd = _data;
      for (int ii = 0; ii < npoints; ii++, dd++) {
	if (*dd != missing) {
	  *dd *= MS_TO_KNOTS;
	}
      }
    }
    break;

  case Params::PASCALS_TO_MBAR:
    {
      fl32 *dd = _data;
      for (int ii = 0; ii < npoints; ii++, dd++) {
	if (*dd != missing) {
	  *dd *= 0.01;
	}
      }
    }
    break;

  case Params::KELVIN_TO_CELSIUS:
    {
      fl32 *dd = _data;
      for (int ii = 0; ii < npoints; ii++, dd++) {
	if (*dd != missing) {
	  *dd -= 273.15;
	}
      }
    }
    break;
    
  case Params::PERCENT_TO_FRACTION:
    {
      fl32 *dd = _data;
      for (int ii = 0; ii < npoints; ii++, dd++) {
	if (*dd != missing) {
	  *dd *= 0.01;
	}
      }
    }
    break;

  default: {}

  } // switch

}

//////////////////////////////////////
// interpolate vlevels as appropriate

void OutputField::interpVlevels()

{

  if (_params.output_level_type == Params::NATIVE_PRESSURE_LEVELS) {
    _useNativeVlevels();
  } else {
    _interpVlevels();
  }
  
}

////////////////////////////
// derive wspd from u and v

int OutputField::deriveWspd(const OutputField &ufld, const OutputField &vfld)

{

  // this only applies to WSPD
  
  if (_gribFieldId != Params::WSPD) {
    cerr << "ERROR - " << _progName << "::OutputField::deriveWspd" << endl;
    cerr << "  deriveWspd() only applies to WSPD field" << endl;
    return -1;
  }

  // check ufld and vfld fields for grid dimensions

  if (ufld._nX != vfld._nX ||
      ufld._nY != vfld._nY ||
      ufld._levels.size() != vfld._levels.size()) {
    cerr << "ERROR - " << _progName << "::OutputField::deriveWspd" << endl;
    cerr << "  UGRD and VGRD fields have different grids" << endl;
    ufld.print(cerr);
    vfld.print(cerr);
    cerr << "=====================================================" << endl;
    return -1;
  }

  // set grid values
  
  _nX = ufld._nX;
  _nY = ufld._nY;
  _dX = ufld._dX;
  _dY = ufld._dY;
  _minX = ufld._minX;
  _minY = ufld._minY;
  _maxX = ufld._maxX;
  _maxY = ufld._maxY;
  _levels = ufld._levels;

  if (_params.debug) {
    print(cerr);
  }

  // load up data array

  int nxy = _nX * _nY;
  int npoints = nxy * _levels.size();
  if (_data) {
    delete[] _data;
  }
  _data = new fl32[npoints];
  fl32 missing = GribMgr::getMissingVal();

  fl32 *dd = _data;
  const fl32 *uu = ufld._data;
  const fl32 *vv = vfld._data;
  for (int ii = 0; ii < npoints; ii++, dd++, uu++, vv++) {
    if (*uu == missing || *vv == missing) {
      *dd = missing;
    } else {
      *dd = (fl32) sqrt(*uu * *uu + *vv * *vv);
    }
    // cerr << "uu, vv, dd: " << *uu << ", " << *vv << ", " << *dd << endl;
  }

  return 0;
  
}

///////////////////////////////////////////
// create MdvxField object from this object
//
// Returns MdvxField* on success, NULL on failure.

MdvxField *OutputField::createMdvxField(time_t gen_time,
					int forecast_lead_time)
  
{

  if (_data == NULL) {
    return NULL;
  }

  // fill out field header

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.field_code = _gribFieldId;
  fhdr.forecast_delta = forecast_lead_time;
  fhdr.forecast_time = gen_time + forecast_lead_time;
  fhdr.nx = _nX;
  fhdr.ny = _nY;
  fhdr.nz = (int) _vlevels.size();
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * fhdr.data_element_nbytes;
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  if (_params.output_level_type == Params::FLIGHT_LEVELS) {
    fhdr.vlevel_type = Mdvx::VERT_FLIGHT_LEVEL;
  } else if (_params.output_level_type == Params::HEIGHT_LEVELS) {
    fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  } else {
    fhdr.vlevel_type = Mdvx::VERT_TYPE_PRESSURE;
  }
  fhdr.dz_constant = false;
  fhdr.proj_origin_lat = _minX;
  fhdr.proj_origin_lon = _minY;
  fhdr.grid_dx = _dX;
  fhdr.grid_dy = _dY;
  fhdr.grid_dz = 1;
  fhdr.grid_minx = _minX;
  fhdr.grid_miny = _minY;
  fhdr.grid_minz = _vlevels[0];
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = GribMgr::getMissingVal();
  fhdr.missing_data_value = fhdr.bad_data_value;

  // fill out vlevel header

  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  for (int ii = 0; ii < fhdr.nz; ii++) {
    vhdr.level[ii] = _vlevels[ii];
    vhdr.type[ii] = fhdr.vlevel_type;
  }
  
  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, _data);

  // set strings

  fld->setFieldName(_name.c_str());
  fld->setFieldNameLong(_longName.c_str());
  fld->setUnits(_units.c_str());
  fld->setTransform("none");

  // set type, compression etc.

  fld->convertDynamic((Mdvx::encoding_type_t) _encoding,
		      (Mdvx::compression_type_t) _params.output_compression);

  return fld;

}
  
//////////////////////////////////////
// using native vlevels - pressure

void OutputField::_useNativeVlevels()

{

  if (_params.debug) {
    cerr << "Using native vlevels - pressure: " << _name << endl;
  }

  _vlevels.clear();
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    _vlevels.push_back((fl32) _levels[ii]);
  }

}

//////////////////////////////////////
// interpolate vertical levels

void OutputField::_interpVlevels()

{

  if (_data == NULL) {
    return;
  }

  IcaoStdAtmos isa;
  vector<double> levelsIn;
  vector<double> levelsOut;
  
  if (_params.output_level_type == Params::FLIGHT_LEVELS) {
    
    if (_params.debug) {
      cerr << "Interpolating onto flight levels, field: " << _name << endl;
    }
    for (size_t ii = 0; ii < _levels.size(); ii++) {
      double flevel = isa.pres2flevel((double) _levels[ii]);
      levelsIn.push_back(flevel);
    }
    for (int ii = 0; ii < _params.flight_levels_n; ii++) {
      levelsOut.push_back((double) _params._flight_levels[ii]);
    }

  } else if (_params.output_level_type == Params::HEIGHT_LEVELS) {

    if (_params.debug) {
      cerr << "Interpolating onto height levels, field: " << _name << endl;
    }
    for (size_t ii = 0; ii < _levels.size(); ii++) {
      // height in km
      double ht = isa.pres2ht((double) _levels[ii]) / 1000.0;
      levelsIn.push_back(ht);
    }
    for (int ii = 0; ii < _params.height_levels_n; ii++) {
      levelsOut.push_back((double) _params._height_levels[ii]);
    }

  } // if (_params.output_level_type ...

  // compute weights
  
  vector<interp_t> interps;
  size_t jstart = 0;
  for (size_t ii = 0; ii < levelsOut.size(); ii++) {
    double thisLevel = levelsOut[ii];
    for (size_t jj = jstart; jj < levelsIn.size() - 1; jj++) {
      if (levelsIn[jj+1] >= thisLevel && levelsIn[jj] <= thisLevel) {
	interp_t intp;
	intp.level = thisLevel;
	intp.ilevel1 = jj;
	intp.ilevel2 = jj + 1;
	double delta = levelsIn[jj+1] - levelsIn[jj];
	intp.wt1 = (levelsIn[jj+1] - thisLevel) / delta;
	intp.wt2 = 1.0 - intp.wt1;
	interps.push_back(intp);
	jstart = jj;
	break;
      }
    } // jj
  } // ii

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    for (size_t ii = 0; ii < interps.size(); ii++) {
      const interp_t &intp = interps[ii];
      cerr << "ii, level, ilevel1, ilevel2, wt1, wt2, valid: "
	   << ii << ", " << intp.level << ", "
	   << intp.ilevel1 << ", " << intp.ilevel2 << ", "
	   << intp.wt1 << ", " << intp.wt2 << endl;
    }
  }

  // set the vlevels
  
  _vlevels.clear();
  for (size_t ii = 0; ii < interps.size(); ii++) {
    _vlevels.push_back((fl32) interps[ii].level);
  }

  // interpolate the data

  int nxy = _nX * _nY;
  int npoints = nxy * _vlevels.size();
  fl32 *interped = new fl32[npoints];
  fl32 missing = GribMgr::getMissingVal();

  for (size_t ii = 0; ii < interps.size(); ii++) {

    double wt1 = interps[ii].wt1;
    double wt2 = interps[ii].wt2;
    fl32 *data1 = _data + interps[ii].ilevel1 * nxy;
    fl32 *data2 = _data + interps[ii].ilevel2 * nxy;
    fl32 *out = interped + ii * nxy;
    
    for (int jj = 0; jj < nxy; jj++, out++, data1++, data2++) {
      if (*data1 == missing || *data2 == missing) {
	*out = missing;
      } else {
	*out = (*data1 * wt1) + (*data2 * wt2);
      }
    } // jj

  } // ii

  // replace native with interpolated data

  delete[] _data;
  _data = interped;

}

//////////////////
// clear the object

void OutputField::clear()

{

  // This object does not own the GridField objects, only the vector
  _fields.clear();

  if (_data) {
    delete[] _data;
    _data = NULL;
  }
  
}

////////
// print

void OutputField::print(ostream &out) const
{

  out << "================ Output Field =======================" << endl;
  out << "  Name: " << _name << endl;
  out << "  LongName: " << _longName << endl;
  out << "  Units: " << _units << endl;
  out << "  GribFieldId: " << _gribFieldId << endl;
  out << "  UnitsConversion: " << _unitsConversion << endl;
  out << "  Encoding: " << _encoding << endl;
  out << "  _nX: " << _nX << endl;
  out << "  _nY: " << _nY << endl;
  out << "  _dX: " << _dX << endl;
  out << "  _dY: " << _dY << endl;
  out << "  _minX: " << _minX << endl;
  out << "  _minY: " << _minY << endl;
  out << "  _maxX: " << _maxX << endl;
  out << "  _maxY: " << _maxY << endl;
  out << "  _levels:";
  for (size_t ii = 0; ii < _levels.size(); ii++) {
    out << " " << _levels[ii];
  }
  out << endl;

}

////////////////////////
// print the grib fields

void OutputField::printInputFields(ostream &out) const

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    out << "Field #: " << ii << endl;
    _fields[ii]->print(out);
  } // ii

}

