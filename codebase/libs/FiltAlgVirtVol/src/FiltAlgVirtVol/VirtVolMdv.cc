/**
 * @file VirtVolMdv.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/VirtVolMdv.hh>
#include <FiltAlgVirtVol/UrlParms.hh>
#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <FiltAlgVirtVol/GriddedData.hh>
#include <Mdv/MdvxField.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
VirtVolMdv::VirtVolMdv(void)
{
}

//------------------------------------------------------------------
VirtVolMdv::~VirtVolMdv(void)
{
}

//------------------------------------------------------------------
void VirtVolMdv::output2d(const time_t &t, const std::vector<Grid2d> &grids,
			  const UrlParms &u)
{
  // special 2d output here
  DsMdvx out;
  
  Mdvx::master_header_t masterHdr = _tweakedMasterHdr(t, 2);
  out.setMasterHeader(masterHdr);
    
  // this seems to work.
  Mdvx::vlevel_header_t vlevelHdr = _vlevelHdr;

  for (size_t i=0; i<grids.size(); ++i)
  {
    string name = grids[i].getName();
    Mdvx::field_header_t fieldHdr = _tweakedFieldHdr(2, _nx, _ny, 1, t, 
						     name, grids[i].getMissing());
    MdvxField *f = new MdvxField(fieldHdr, vlevelHdr, (void *)0, true, false);
    fl32 *fo = (fl32 *)f->getVol();
    for (int j=0; j<_nx*_ny; ++j)
    {
      double value;
      if (grids[i].getValue(j, value))
      {
	fo[j] = value;
      }
      else
      {
	fo[j] = grids[i].getMissing();
      }
    }
    f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
    out.addField(f);
  }
  if (u.is_netCDF_output)
  {
    for (size_t i=0; i<_chunks.size(); ++i)
    {
      MdvxChunk *c = new MdvxChunk(_chunks[i]);
      out.addChunk(c);
    }
    out.setWriteFormat(Mdvx::FORMAT_NCF);
  }
  out.setWriteLdataInfo();
  if (out.writeToDir(u.url))
  {
    LOG(ERROR) << "Unable to write mdv to " << u.url;
  }
  else
  {
    LOG(DEBUG) << "Wrote data to " << u.url;
  }
}

//------------------------------------------------------------------
bool VirtVolMdv::initialize(const time_t &t, const UrlParms &u,
			    const std::string &fieldName, const FiltAlgParms *parms)
{
  LOG(DEBUG) << " creating initial input state using " << u.url;
  DsMdvx mdv;
  mdv.setReadTime(Mdvx::READ_CLOSEST, u.url, 0, t);
  mdv.addReadField(fieldName);
  if (parms->restrict_vertical_levels)
  {
    mdv.setReadVlevelLimits(parms->_vertical_level_range[0],
			    parms->_vertical_level_range[1]);
  }

  if (mdv.readVolume())
  {
    LOG(ERROR)<< "reading volume " << u.url;
    LOG(ERROR) << mdv.getErrStr();
    return false;
  }
  
  MdvxField *f = mdv.getFieldByName(fieldName);
  if (f == NULL)
  {
    LOG(ERROR) << "reading field " << fieldName << " from " <<  u.url;
    return  false;
  }
  _masterHdr = mdv.getMasterHeader();
  Mdvx::field_header_t hdr = f->getFieldHeader();
  _nz = hdr.nz;
  _ny = hdr.ny;
  _nx = hdr.nx;
  if (parms->restrict_max_range)
  {
    _nx = parms->max_range;
    _fieldHdr = hdr;
    _fieldHdr.nx = parms->max_range;
    _fieldHdr.volume_size = _fieldHdr.nx *  _fieldHdr.ny * 
      _fieldHdr.nz * _fieldHdr.data_element_nbytes;
    _proj = MdvxProj(_masterHdr, _fieldHdr);
  }
  else
  {
    _proj = MdvxProj(_masterHdr, hdr);
    _fieldHdr = hdr;
  }

  int nc = mdv.getNChunks();
  for (int i=0; i<nc; ++i)
  {
    MdvxChunk *c = mdv.getChunkByNum(i);
    if (c->getId() == Mdvx::CHUNK_DSRADAR_CALIB ||
	c->getId() == Mdvx::CHUNK_DSRADAR_PARAMS ||
	c->getId() == Mdvx::CHUNK_DSRADAR_ELEVATIONS)
    {
      // for later conversion to netCDF
      _chunks.push_back(MdvxChunk(*c));
    }
  }

  _vlevelHdr = f->getVlevelHeader();

  Mdvx::coord_t coord = _proj.getCoord();
  _positiveDy = coord.dy > 0.0;
  MdvxRadar mdvRadar;
  if (mdvRadar.loadFromMdvx(mdv) && mdvRadar.radarParamsAvail())
  {
    _hasWavelength = true;
    _hasAltitude = true;
    DsRadarParams &rparams = mdvRadar.getRadarParams();
    _wavelength = rparams.wavelength;
    _altitude = rparams.altitude;
  }
  else
  {
    _hasWavelength = false;
    _hasAltitude = true;
    _altitude = _masterHdr.sensor_alt;
  }

  _vlevel.clear();
  for (int i=0; i<_nz; ++i)
  {
    _vlevel.push_back(_vlevelHdr.level[i]);
  }
  return true;
}

//------------------------------------------------------------------
bool VirtVolMdv::readAllFields(const time_t &t, const std::string &url,
			       const std::vector<std::string> &fields,
			       const FiltAlgParms *parms)
{
  LOG(DEBUG) << " creating input for " << url;
  // clear out anything in this object
  _mdvRead = DsMdvx();
  _mdvRead.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, t);
  for (size_t i=0; i<fields.size(); ++i)
  {
    LOG(DEBUG) << " data = " << fields[i];
    _mdvRead.addReadField(fields[i]);
  }
  if (parms->restrict_vertical_levels)
  {
    _mdvRead.setReadVlevelLimits(parms->_vertical_level_range[0],
				 parms->_vertical_level_range[1]);
  }
  LOG(DEBUG) << "Reading";
  if (_mdvRead.readVolume())
  {
    LOG(ERROR)<< "reading volume " << url;
    return false;
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------
std::vector<GriddedData>
VirtVolMdv::initializeInputField(const std::string &field,
				 const std::string &url,
				 const FiltAlgParms *parms)
{
  vector<GriddedData> ret;
  MdvxField *f = _mdvRead.getFieldByName(field);
  if (f == NULL)
  {
    LOG(ERROR) << "reading field " << field << " from " << url;
    return ret;
  }
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  Mdvx::field_header_t hdr = f->getFieldHeader();
  if (_nz != hdr.nz || _ny != hdr.ny)
  {
    LOG(ERROR) << "Unequal dimensions in z or y";
    return ret;
  }
  if (!parms->restrict_max_range)
  {
    if (_nx != hdr.nx)
    {
      LOG(ERROR) << "Unequal dimensions in x";
      return ret;
    }
  }

  for (int j=0; j<hdr.nz; ++j)
  {
    Grid2d g(field, _nx, hdr.ny, hdr.missing_data_value);
    GriddedData gd(g);
    fl32 *data = (fl32 *)f->getVol();

    // use input data x dimension always, and downsample
    int k0 = hdr.nx*hdr.ny*j;  
    for (int iy=0; iy<hdr.ny ; ++iy)
    {
      for (int ix=0; ix<_nx; ++ix)
      {
	int k = iy*hdr.nx + ix;
	if (data[k0+k] != hdr.bad_data_value &&
	    data[k0+k] != hdr.missing_data_value)
	{
	  gd.setValue(ix, iy, static_cast<double>(data[k0+k]));
	}
      }
    }
    ret.push_back(gd);
  }
  return ret;
}

//------------------------------------------------------------------
void VirtVolMdv::initializeOutput(const time_t &t, int nz)
{
  _mdvWrite = DsMdvx();
  Mdvx::master_header_t masterHdr = _tweakedMasterHdr(t, nz);
  _mdvWrite.setMasterHeader(masterHdr);
}

//------------------------------------------------------------------
void VirtVolMdv::addOutputField(const time_t &t, const std::string &name,
				const std::vector<Grid2d> &data3d,
				double missing)
{
  if ((int)data3d.size() != _nz)
  {
    LOG(ERROR) << "Inconsistent number of vertical levels " << data3d.size()
	       << " "<<  _nz;
    return;
  }
  
  Mdvx::field_header_t fieldHdr = _tweakedFieldHdr(3, _nx, _ny, _nz, t, 
						   name,  missing);
  MdvxField *f = new MdvxField(fieldHdr, _vlevelHdr, (void *)0,true,false);
  fl32 *fo = (fl32 *)f->getVol();

  // Loop through and populate the new field
  for ( size_t z=0; z<data3d.size(); ++z)
  {
    for (int i=0; i<_nx*_ny; ++i)
    {
      fo[i+z*_nx*_ny] = data3d[z].getValue(i);
    }
  }
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
  _mdvWrite.addField(f);
}

//------------------------------------------------------------------
void VirtVolMdv::write(const UrlParms &u)
{
  if (u.data_type == UrlParams::GRID)
  {
    if (u.is_netCDF_output)
    {
      for (size_t i=0; i<_chunks.size(); ++i)
      {
	MdvxChunk *c = new MdvxChunk(_chunks[i]);
	_mdvWrite.addChunk(c);
      }
      _mdvWrite.setWriteFormat(Mdvx::FORMAT_NCF);
    }
    _mdvWrite.setWriteLdataInfo();
    if (_mdvWrite.writeToDir(u.url))
    {
      LOG(ERROR) << "Unable to write mdv to " << u.url;
    }
    else
    {
      LOG(DEBUG) << "Wrote data to " << u.url;
    }
    // clear memory
    _mdvWrite = DsMdvx();
  }
}

//------------------------------------------------------------------
int VirtVolMdv::getNGates(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.nx;
}  

//------------------------------------------------------------------
double VirtVolMdv::getDeltaGateKm(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.dx;
}

//------------------------------------------------------------------
double VirtVolMdv::getDeltaAzDeg(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.dy;
}

//------------------------------------------------------------------
double VirtVolMdv::getStartRangeKm(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.minx;
}

//------------------------------------------------------------------
int VirtVolMdv::nz(void) const
{
  return _nz;
}

//------------------------------------------------------------------
void VirtVolMdv::getRadarParams(bool &hasWavelength, bool &hasAltitude,
				double &altitude, double &wavelength) const
{
  hasWavelength = _hasWavelength;
  hasAltitude = _hasAltitude;
  altitude = _altitude;
  wavelength = _wavelength;
}

//----------------------------------------------------------------------
double VirtVolMdv::extractAltitude(void) const
{
  if (!_hasAltitude)
  {
    LOG(WARNING) << " Data does not have sensor altitude, assume 0.0";
    return 0.0;
  }
  else
  {
    return _altitude;  // should be km
  }
}

//------------------------------------------------------------------
Mdvx::master_header_t VirtVolMdv::_tweakedMasterHdr(const time_t &t,
						    int dataDimension) const
{
  Mdvx::master_header_t masterHdr = _masterHdr;
  masterHdr.time_gen = masterHdr.time_end = masterHdr.time_centroid = t;
  masterHdr.time_expire = t;
  masterHdr.forecast_time = t;
  masterHdr.forecast_delta = 0;
  masterHdr.data_dimension = dataDimension;
  return masterHdr;
}

//------------------------------------------------------------------
Mdvx::field_header_t
VirtVolMdv::_tweakedFieldHdr(int dataDimension, int nx, int ny, int nz,
			     const time_t &t,  const std::string &name,
			     double badValue) const
{
  Mdvx::field_header_t fieldHdr = _fieldHdr;
  fieldHdr.dz_constant = 1;
  fieldHdr.data_dimension = dataDimension;
  fieldHdr.nz = nz;
  fieldHdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fieldHdr.data_element_nbytes = 4;
  fieldHdr.volume_size = nx*ny*nz*fieldHdr.data_element_nbytes;
  fieldHdr.compression_type = Mdvx::COMPRESSION_NONE; // done later
  fieldHdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fieldHdr.scaling_type = Mdvx::SCALING_DYNAMIC;
  fieldHdr.scale = 1.0;
  fieldHdr.bias = 0.0;
  fieldHdr.forecast_delta = 0;
  fieldHdr.forecast_time = t;
  strncpy(fieldHdr.units, "units", MDV_UNITS_LEN-1);
  fieldHdr.units[MDV_UNITS_LEN-1] = '\0';
  fieldHdr.bad_data_value = badValue;
  fieldHdr.missing_data_value = badValue;
  strncpy(fieldHdr.field_name_long, name.c_str(), MDV_LONG_FIELD_LEN-1);
  fieldHdr.field_name_long[MDV_LONG_FIELD_LEN-1] = '\0';
  strncpy(fieldHdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN-1);
  fieldHdr.field_name[MDV_SHORT_FIELD_LEN-1] = '\0';
  return fieldHdr;
}

