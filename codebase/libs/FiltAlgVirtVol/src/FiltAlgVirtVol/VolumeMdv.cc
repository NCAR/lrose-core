/**
 * @file VolumeMdv.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/VolumeMdv.hh>
#include <FiltAlgVirtVol/SweepMdv.hh>
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
VolumeMdv::VolumeMdv(void) :
  _archiveChecked(false), _isArchiveMode(false), _debug(false), _T(NULL),
  _parms(NULL), _special(NULL)
{
}

//------------------------------------------------------------------
VolumeMdv::VolumeMdv(const FiltAlgParms *parms, int argc, char **argv):
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
VolumeMdv::~VolumeMdv(void)
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
bool VolumeMdv::triggerMdv(time_t &t)
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
void VolumeMdv::clear(void)
{
  _data.clear();
}

//------------------------------------------------------------------
const std::vector<GriddedData> *VolumeMdv::get2d(int index) const
{
  return &(_data[index]._grid2d);
}

//------------------------------------------------------------------
std::vector<GriddedData> VolumeMdv::getField3d(const std::string &name) const
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
void VolumeMdv::addNewMdv(int zIndex, const SweepMdv &s)
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
void VolumeMdv::addNewGrid(int zIndex, const GriddedData &g)
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
bool VolumeMdv::storeMathUserDataMdv(const std::string &name, MathUserData *v)
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
void VolumeMdv::output(const time_t &t)
{
  // for each output url
  for (size_t i=0; i<_parms->_virtvol_outputs.size(); ++i)
  {
    _outputToUrl(t, _parms->_virtvol_outputs[i]);
  }
}

//------------------------------------------------------------------
int VolumeMdv::numProcessingNodes(bool twoD) const
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
int VolumeMdv::getNGates(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.nx;
}  

//------------------------------------------------------------------
double VolumeMdv::getDeltaGateKm(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.dx;
}

//------------------------------------------------------------------
double VolumeMdv::getDeltaAzDeg(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.dy;
}

//------------------------------------------------------------------
double VolumeMdv::getStartRangeKm(void) const
{
  Mdvx::coord_t coord = _proj.getCoord();
  return coord.minx;
}

//------------------------------------------------------------------
int VolumeMdv::nz(void) const
{
  return _nz;
}

//------------------------------------------------------------------
bool VolumeMdv::_initialInitializeInput(const time_t &t, const UrlSpec &u)
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
  Mdvx::field_header_t hdr = f->getFieldHeader();
  _nz = hdr.nz;
  _nx = hdr.nx;
  _ny = hdr.ny;
  _data.clear();
  for (int i=0; i<_nz; ++i)
  {
    _data.push_back(GridFields(i));
  }
  _masterHdr = mdv.getMasterHeader();
  _fieldHdr = hdr;
  _vlevelHdr = f->getVlevelHeader();
  _proj = MdvxProj(_masterHdr, hdr);
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
bool VolumeMdv::_initializeInput(const time_t &t, const UrlSpec &u)
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
      if (_nz != hdr.nz || _nx != hdr.nx || _ny != hdr.ny)
      {
	LOG(ERROR) << "Unequal dimensions";
	return false;
      }
      for (int j=0; j<hdr.nz; ++j)
      {
	Grid2d g(fields[i]._internal, hdr.nx, hdr.ny, hdr.missing_data_value);
	GriddedData gd(g);
	fl32 *data = (fl32 *)f->getVol();
	int k0 = hdr.nx*hdr.ny*j;
	for (int k=0; k<hdr.nx*hdr.ny ; ++k)
	{
	  if (data[k0+k] != hdr.bad_data_value &&
	      data[k0+k] != hdr.missing_data_value)
	  {
	    gd.setValue(k, static_cast<double>(data[k0+k]));
	  }
	}
	_data[j]._grid2d.push_back(gd);
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
void VolumeMdv::_outputToUrl(const time_t &t, const UrlSpec &u)
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
void VolumeMdv::_outputFieldToUrl(const std::string &internalName,
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
