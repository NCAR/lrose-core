/**
 * @file VirtVolVolume.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <FiltAlgVirtVol/VirtVolSweep.hh>
#include <rapmath/UnaryNode.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <dsdata/DsUrlTrigger.hh>

#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>

#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <cstdlib>

//------------------------------------------------------------------
VirtVolVolume::VirtVolVolume(void) :
  _archiveChecked(false), _isArchiveMode(false), _debug(false), _T(NULL),
  _parms(NULL), _special(NULL)
{
}

//------------------------------------------------------------------
VirtVolVolume::VirtVolVolume(const FiltAlgParms *parms,
			     int argc, char **argv):
  _archiveChecked(false), _isArchiveMode(false), _debug(false), _T(NULL),
  _parms(parms), _special(NULL)
{
  bool error;
  DsUrlTrigger::checkArgs(argc, argv, _archiveT0, _archiveT1, _isArchiveMode,
			  error);
  if (error)
  {
    LOG(ERROR) << "ERROR parsing args";
    exit(1);
  }
  _archiveChecked = true;
  _debug = parms->debug_triggering;
  if (_debug)
  {
    LogMsgStreamInit::setThreading(true);
  }
  LOG(DEBUG_VERBOSE) << "------before trigger-----";

  if (_isArchiveMode)
  {
    DsTimeListTrigger *ltrigger = new DsTimeListTrigger();
    if (ltrigger->init(parms->trigger_url, _archiveT0, _archiveT1) != 0)
    {
      LOG(ERROR) << "Initializing triggering";
      LOG(ERROR) << ltrigger->getErrStr();
      _T = NULL;
      delete ltrigger;
    }
    else
    {
      _T = ltrigger;
    }
  }
  else
  {
    DsLdataTrigger *ltrigger =new DsLdataTrigger();
    if (ltrigger->init(parms->trigger_url, 30000, PMU_auto_register) != 0)
    {
      LOG(ERROR) << "Initializing triggering";
      _T = NULL;
      delete ltrigger;
    }
    else
    {
      _T = ltrigger;
    }
  }
}

//------------------------------------------------------------------
VirtVolVolume::~VirtVolVolume(void)
{
  if (_T != NULL)
  {
    delete _T;
    _T = NULL;
  }
  if (_special != NULL)
  {
    delete _special;
  }
}

//------------------------------------------------------------------
bool VirtVolVolume::triggerVirtVol(time_t &t)
{
  if (_T == NULL)
  {
    LOG(DEBUG) << "No triggering";
    return false;
  }
  
  if (_T->endOfData())
  {
    LOG(DEBUG) << "no more triggering";
    delete _T;
    _T = NULL;
    return false;
  }
  DateTime tt;
  if (_T->nextIssueTime(tt) != 0)
  {
    LOG(ERROR) << "Getting next trigger time";
    return true;
  }
  _time = tt.utime();
  t = _time;
    LOG(DEBUG) << "-------Triggered " << DateTime::strn(t) << " ----------";
  if (_special != NULL)
  {
    delete _special;
  }
  _special = new SpecialUserData();

  // try to initialize state using any of the inputs
  bool didInit = false;
  for (size_t i=0; i<_parms->_virtvol_inputs.size(); ++i)
  {
    if (_initialInitializeInput(_time, _parms->_virtvol_inputs[i]))
    {
      didInit = true;
      break;
    }
  }
  if (!didInit)
  {
    LOG(ERROR) << "Could not init";
    return false;
  }

  // now load in volume data for each url
  for (size_t i=0; i<_parms->_virtvol_inputs.size(); ++i)
  {
    if (!_initializeInput(_time, _parms->_virtvol_inputs[i]))
    {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------
void VirtVolVolume::clear(void)
{
  _data.clear();
}

//------------------------------------------------------------------
const std::vector<GriddedData> *VirtVolVolume::get2d(int index) const
{
  return &(_data[index]._grid2d);
}

//------------------------------------------------------------------
std::vector<GriddedData>
VirtVolVolume::getField3d(const std::string &name) const
{
  std::vector<GriddedData> ret;
  
  for (size_t i=0; i<_data.size(); ++i)
  {
    bool got = false;
    for (int j=0; j<_data[i].num(); ++j)
    {
      if (_data[i].ithGrid(j)->nameEquals(name))
      {
	got = true;
	ret.push_back(*(_data[i].ithGrid(j)));
	break;
      }
    }
    if (!got)
    {
      LOG(ERROR) << "No " << name << " at height = " << i;
      ret.clear();
      return ret;
    }
  }

  return ret;
}

//------------------------------------------------------------------
void VirtVolVolume::addNewSweep(int zIndex, const VirtVolSweep &s)
{
  const std::vector<GriddedData> &newD = s.newDataRef();
  for (size_t i=0; i<newD.size(); ++i)
  {
    string name = newD[i].getName();
    string ename;
    if (_parms->outputInternal2ExternalName(name, ename))
    {
      bool exists = false;
      for (size_t j=0; j<_data[zIndex]._grid2d.size(); ++j)
      {
	if (name == _data[zIndex]._grid2d[j].getName())
	{
	  exists = true;
	  break;
	}
      }
      if (!exists)
      {
	LOG(DEBUG) << "Adding field " << name << " to state, z=" << zIndex;
	_data[zIndex]._grid2d.push_back(newD[i]);
      }
    }
  }
}

//------------------------------------------------------------------
void VirtVolVolume::addNewGrid(int zIndex, const GriddedData &g)
{
  string name = g.getName();
  string ename;
  if (_parms->outputInternal2ExternalName(name, ename))
  {
    bool exists = false;
    for (size_t j=0; j<_data[zIndex]._grid2d.size(); ++j)
    {
      if (name == _data[zIndex]._grid2d[j].getName())
      {
	exists = true;
	break;
      }
    }
    if (!exists)
    {
      LOG(DEBUG) << "Adding field " << name << " to state, z=" << zIndex;
      _data[zIndex]._grid2d.push_back(g);
    }
    else
    {
      LOG(ERROR) << "Can't duplicate field " << name
		 << " to state z=" << zIndex;
    }
  }
}

//------------------------------------------------------------------
bool VirtVolVolume::storeMathUserDataVirtVol(const std::string &name,
					     MathUserData *v)
{
  if (_special == NULL)
  {
    LOG(ERROR) << "Pointer not set";
    return false;
  }
  else
  {
    return _special->store(name, v);
  }
}

//------------------------------------------------------------------
void VirtVolVolume::output(const time_t &t)
{
  // for each output url
  for (size_t i=0; i<_parms->_virtvol_outputs.size(); ++i)
  {
    _outputToUrl(t, _parms->_virtvol_outputs[i]);
  }
}

//------------------------------------------------------------------
int VirtVolVolume::numProcessingNodes(bool twoD) const
{
  if (twoD)
  {
    return (int)(_data.size());
  }
  else
  {
    return 0;
  }
}

//------------------------------------------------------------------
// virtual
bool VirtVolVolume::
synchUserDefinedInputs(const std::string &userKey,
		       const std::vector<std::string> &names)
{
  return virtVolSynchUserInputs(userKey, names);
}

//------------------------------------------------------------------
int VirtVolVolume::getNGates(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.nx;
}  

//------------------------------------------------------------------
double VirtVolVolume::getDeltaGateKm(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.dx;
}

//------------------------------------------------------------------
double VirtVolVolume::getDeltaAzDeg(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.dy;
}

//------------------------------------------------------------------
double VirtVolVolume::getStartRangeKm(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.minx;
}

//------------------------------------------------------------------
int VirtVolVolume::nz(void) const
{
  return _nz;
}

//------------------------------------------------------------------
bool
VirtVolVolume::_initialInitializeInput(const time_t &t, const UrlSpec &u)
{
  vector<NamePair> fields = u.fieldNames();
  if (fields.empty())
  {
    return false;
  }

  LOG(DEBUG) << " creating initial input state using " << u._url;
  DsMdvx mdv;
  mdv.setReadTime(Mdvx::READ_CLOSEST, u._url, 0, t);
  mdv.addReadField(fields[0]._external);
  if (_parms->restrict_vertical_levels)
  {
    mdv.setReadVlevelLimits(_parms->_vertical_level_range[0],
			    _parms->_vertical_level_range[1]);
  }

  LOG(DEBUG) << "Reading";
  if (mdv.readVolume())
  {
    LOG(ERROR)<< "reading volume " << u._url;
    LOG(ERROR) << mdv.getErrStr();
    return false;
  }
  MdvxField *f = mdv.getFieldByName(fields[0]._external);
  if (f == NULL)
  {
    LOG(ERROR) << "reading field " << fields[0]._external << " from " <<
      u._url;
    return  false;
  }
  _masterHdr = mdv.getMasterHeader();
  Mdvx::field_header_t hdr = f->getFieldHeader();
  _nz = hdr.nz;
  _ny = hdr.ny;
  _nx = hdr.nx;
  if (_parms->restrict_max_range)
  {
    _nx = _parms->max_range;
    _fieldHdr = hdr;
    _fieldHdr.nx = _parms->max_range;
    _fieldHdr.volume_size = _fieldHdr.nx *  _fieldHdr.ny * 
      _fieldHdr.nz * _fieldHdr.data_element_nbytes;
    _proj = MdvxProj(_masterHdr, _fieldHdr);
  }
  else
  {
    _proj = MdvxProj(_masterHdr, hdr);
    _fieldHdr = hdr;
  }
  _data.clear();
  for (int i=0; i<_nz; ++i)
  {
    _data.push_back(GridFields(i));
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
bool VirtVolVolume::_initializeInput(const time_t &t, const UrlSpec &u)
{
  vector<NamePair> fields = u.fieldNames();
  if (!fields.empty())
  {
    LOG(DEBUG) << " creating input for " << u._url;
    DsMdvx mdv;
    mdv.setReadTime(Mdvx::READ_FIRST_BEFORE, u._url, 0, t);
    for (size_t i=0; i<fields.size(); ++i)
    {
      LOG(DEBUG) << " data = " << fields[i]._external;
      mdv.addReadField(fields[i]._external);
    }
    if (_parms->restrict_vertical_levels)
    {
      mdv.setReadVlevelLimits(_parms->_vertical_level_range[0],
			      _parms->_vertical_level_range[1]);
    }
    LOG(DEBUG) << "Reading";
    if (mdv.readVolume())
    {
      LOG(ERROR)<< "reading volume " << u._url;
      return false;
    }

    for (size_t i=0; i<fields.size(); ++i)
    {
      LOG(DEBUG) << "Storing locally " << fields[i]._external;
      MdvxField *f = mdv.getFieldByName(fields[i]._external);
      if (f == NULL)
      {
	LOG(ERROR) << "reading field " << fields[i]._external << " from " <<
	  u._url;
	return  false;
      }
      f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
      Mdvx::field_header_t hdr = f->getFieldHeader();
      if (_nz != hdr.nz || _ny != hdr.ny)
      {
	LOG(ERROR) << "Unequal dimensions in z or y";
	return false;
      }
      if (!_parms->restrict_max_range)
      {
	if (_nx != hdr.nx)
	{
	  LOG(ERROR) << "Unequal dimensions in x";
	  return false;
	}
      }

      for (int j=0; j<hdr.nz; ++j)
      {
	Grid2d g(fields[i]._internal, _nx, hdr.ny, hdr.missing_data_value);
	GriddedData gd(g);
	fl32 *data = (fl32 *)f->getVol();
	// use input data x dimension always,
	// and downsample
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
	_data[j]._grid2d.push_back(gd);
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
void VirtVolVolume::_outputToUrl(const time_t &t, const UrlSpec &u)
{
  vector<NamePair> names = u.fieldNames();

  if (!names.empty())
  {
    //construct each named field

    DsMdvx out;
  
    _masterHdr.time_gen = _masterHdr.time_end = _masterHdr.time_centroid = t;
    _masterHdr.time_expire = t;
    _masterHdr.forecast_time = t;
    _masterHdr.forecast_delta = 0;
    out.setMasterHeader(_masterHdr);
    for (size_t i=0; i<names.size(); ++i)
    {
      _outputFieldToUrl(names[i]._internal, names[i]._external, t, out);
    }
    out.setWriteLdataInfo();
    if (out.writeToDir(u._url))
    {
      LOG(ERROR) << "Unable to write mdv";
    }
    else
    {
      LOG(DEBUG) << "Wrote data to " << u._url;
    }
  }

  names = u.valueNames();
  for (size_t i=0; i<names.size(); ++i)
  {
  }
}

//------------------------------------------------------------------
void VirtVolVolume::_outputFieldToUrl(const std::string &internalName,
				      const std::string &externalName,
				      const time_t &t, DsMdvx &out)
{
  _fieldHdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _fieldHdr.data_element_nbytes = 4;
  _fieldHdr.volume_size = _nx*_ny*_nz*_fieldHdr.data_element_nbytes;
  _fieldHdr.compression_type = Mdvx::COMPRESSION_NONE; // done later
  _fieldHdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fieldHdr.scaling_type = Mdvx::SCALING_DYNAMIC;
  _fieldHdr.scale = 1.0;
  _fieldHdr.bias = 0.0;
  _fieldHdr.forecast_delta = 0;
  _fieldHdr.forecast_time = t;
  strncpy(_fieldHdr.field_name_long, externalName.c_str(),
	  MDV_LONG_FIELD_LEN-1);
  _fieldHdr.field_name_long[MDV_LONG_FIELD_LEN-1] = '\0';
  strncpy(_fieldHdr.field_name, externalName.c_str(), MDV_SHORT_FIELD_LEN-1);
  _fieldHdr.field_name[MDV_SHORT_FIELD_LEN-1] = '\0';
  strncpy(_fieldHdr.units, "units", MDV_UNITS_LEN-1);
  _fieldHdr.units[MDV_UNITS_LEN-1] = '\0';

  // Try to get some sample data so as to set the missing values
  bool got = false;
  for ( size_t z=0; z<_data.size(); ++z)
  {
    for (size_t f=0; f<_data[z]._grid2d.size(); ++f)
    {
      if (_data[z]._grid2d[f].getName() == internalName)
      {
	got = true;
	_fieldHdr.bad_data_value = _data[z]._grid2d[f].getMissing();
	_fieldHdr.missing_data_value = _fieldHdr.bad_data_value;
	break;
      }
    }
    if (got)
    {
      break;
    }
  }
  if (!got)
  {
    LOG(ERROR) << "Missing data " << internalName;
    LOG(WARNING) << "Write some code to fill missing";
    return;
  }

  // finally, add the field
  MdvxField *f = new MdvxField(_fieldHdr, _vlevelHdr, (void *)0,true,false);
  fl32 *fo = (fl32 *)f->getVol();

  // Loop through again and populate the new field
  for ( size_t z=0; z<_data.size(); ++z)
  {
    for (size_t f=0; f<_data[z]._grid2d.size(); ++f)
    {
      if (_data[z]._grid2d[f].getName() == internalName)
      {
	for (int i=0; i<_nx*_ny; ++i)
	{
	  fo[i+z*_nx*_ny] = _data[z]._grid2d[f].getValue(i);
	}
	break;
      }
    }
  }
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
  out.addField(f);
}
