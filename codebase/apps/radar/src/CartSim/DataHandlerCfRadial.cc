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
 * @file DataHandlerCfRadial.cc
 */
#include "DataHandlerCfRadial.hh"
#include "Data.hh"
#include "Fields.hh"
#include "SimMath.hh"

#ifndef NO_NETCDF
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/NcfRadxFile.hh>
#endif
#include <didss/DataFileNames.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/LogMsg.hh>

#ifndef NO_NETCDF
//------------------------------------------------------------------
static string nameWithoutPath(const string &name)
{
  string::size_type i = name.find_last_of("/");
  string ret = name.substr(i+1);
  return ret;
}
#endif

//----------------------------------------------------------------
DataHandlerCfRadial::DataHandlerCfRadial(const Params &parms) :
  DataHandler(parms)
{
  _polar = true;
}

//----------------------------------------------------------------
DataHandlerCfRadial::~DataHandlerCfRadial()
{
}

//----------------------------------------------------------------
DataHandlerCfRadial::DataHandlerCfRadial(const DataHandlerCfRadial &d) :
  DataHandler(d)
#ifndef NO_NETCDF
  ,
  _vol(d._vol)
#endif
{
  if (!_setRays())
  {
    LOG(LogMsg::FATAL, "Cannot set rays\n");
    exit(1);
  }
  reset();
}

//----------------------------------------------------------------
DataHandler *DataHandlerCfRadial::clone(void) const
{
  DataHandlerCfRadial *r = new DataHandlerCfRadial(*this);
  return (DataHandler *)r;
}

//----------------------------------------------------------------
bool DataHandlerCfRadial::init(void)
{
#ifdef NO_NETCDF
  LOG(LogMsg::ERROR, "NetCDF not supported");
  return false;
#else
  RadxTimeList tlist;
  DateTime dt(_parms._template_time[0], _parms._template_time[1],
	      _parms._template_time[2], _parms._template_time[3],
	      _parms._template_time[4], _parms._template_time[5]);
  time_t ttime = dt.utime();
  tlist.setModeFirstBefore(ttime, 10000);
  if (tlist.compile())
  {
    LOGF(LogMsg::ERROR, "Cannot compile time list, dir: %s",
	 _parms.template_url);
    LOG(LogMsg::ERROR, tlist.getErrStr().c_str());
    return false;
  }
  vector<string> paths = tlist.getPathList();
  if (paths.empty())
  {
    LOG(LogMsg::ERROR, "No input data in time range specified");
    return false;
  }

  string path = *(paths.rbegin());
  string name = nameWithoutPath(path);
  LOGF(LogMsg::DEBUG, "Template file: %s", name.c_str());
  
  RadxFile primaryFile;
  primaryFile.addReadField(_parms.template_url_field);
  if (primaryFile.readFromPath(path, _vol))
  {
    LOGF(LogMsg::ERROR, "Cannot read in primary file: %s", name.c_str());
    return false;
  }
  time_t t = _vol.getEndTimeSecs();
  bool dateOnly;
  if (DataFileNames::getDataTime(path, t, dateOnly))
  {
    LOGF(LogMsg::ERROR, "Cannot get time from file path: %s", name.c_str());
    return false;
  }

  LOGF(LogMsg::DEBUG, "-------Triggered %s ----------", 
       RadxTime::strm(t).c_str());

  if (!_setRays())
  {
    return false;
  }
    
  _reset();
  return true;
#endif
}

//----------------------------------------------------------------
void DataHandlerCfRadial::reset(void)
{
  _reset();
}

//----------------------------------------------------------------
void DataHandlerCfRadial::store(const Data &data)
{
#ifdef NO_NETCDF
  LOG(LogMsg::ERROR, "Not implemented");
#else
  for (int i=0; i<_parms.fields_n; ++i)
  {
    Params::Field_t f = _parms._fields[i];
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

    RadxField *field = _rays[_iray]->getField(Fields::fieldName(f));
    Radx::fl32 *data = (Radx::fl32 *)field->getDataFl32();
    if (motion)
    {
      v /= KNOTS_TO_MS;
    }
    data[_j] = v;
  }
#endif
}

//----------------------------------------------------------------
void DataHandlerCfRadial::write(const time_t &currentTime)
{
#ifdef NO_NETCDF
  LOG(LogMsg::ERROR, "Not implemented");
#else
  for (size_t i = 0; i<_rays.size(); ++i)
  {
    RadxRay *ray = _rays[i];
    ray->setTime(currentTime, 0);
  }

  _vol.setStartTime(currentTime, 0);
  _vol.setEndTime(currentTime +
		  static_cast<int>(_parms.simulation_step_minutes*30), 0);
  _vol.loadVolumeInfoFromRays();
  _vol.loadSweepInfoFromRays();
  _vol.setPackingFromRays();

  RadxFile *outFile;
  NcfRadxFile *ncfFile = new NcfRadxFile();
  outFile = ncfFile;
  ncfFile->setNcFormat(RadxFile::NETCDF_CLASSIC);
  outFile->setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  outFile->setCompressionLevel(4);
  outFile->setWriteNativeByteOrder(false);
  outFile->setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  if (outFile->writeToDir(_vol, _parms.output_url, true, false))
  {
    LOGF(LogMsg::ERROR, "Cannot write file to dir: %s", _parms.output_url);
    delete outFile;
    return;
  }
  string outputPath = outFile->getPathInUse();
  delete outFile;

  string name = nameWithoutPath(outputPath);
  LOGF(LogMsg::DEBUG, "Wrote output file: %s", name.c_str());
  return;
#endif
}

//----------------------------------------------------------------
void DataHandlerCfRadial::_initIndexing(void)
{
  _iray = 0;
  _j = 0;
}

//----------------------------------------------------------------
Xyz DataHandlerCfRadial::_computeXyzMeters(void)
{
#ifdef NO_NETCDF
  return Xyz();
#else
  _cx = (_rays[_iray]->getGateSpacingKm()*_j +
	 _rays[_iray]->getStartRangeKm())*1000.0;
  _cy = 90.0 - _rays[_iray]->getAzimuthDeg();
  _cz = _rays[_iray]->getElevationDeg();
  while (_cy < 0)
  {
    _cy += 360;
  }
  while (_cy > 360.0)
  {
    _cy -= 360;
  }
  _cy *= 3.14159/180.0;
  _cz *= 3.14159/180.0;
  return Xyz(_cx*cos(_cy)*cos(_cz), _cx*sin(_cy)*cos(_cz), _cx*sin(_cz));
#endif
}

//----------------------------------------------------------------
bool DataHandlerCfRadial::_increment(void)
{
#ifdef NO_NETCDF
  return false;
#else
  if (++_j >= (int)_rays[_iray]->getNGates())
  {
    _j = 0;
    if (++_iray >= (int)_rays.size())
    {
      return false;
    }
  }
  return true;
#endif
}


//----------------------------------------------------------------
bool DataHandlerCfRadial::_setRays(void)
{
#ifdef NO_NETCDF
  return false;
#else
  _rays = _vol.getRays();
  if (_rays.empty())
  {
    LOG(LogMsg::ERROR, "No rays in volume");
    return false;
  }
  for (size_t i = 0; i<_rays.size(); ++i)
  {
    RadxRay *ray = _rays[i];
    int npt = ray->getNGates();
    Radx::fl32 *data = new Radx::fl32[npt];
    vector<string> wanted;
    for (int j=0; j<_parms.fields_n; ++j)
    {
      string name = Fields::fieldName(_parms._fields[j]);
      string units = Fields::fieldUnits(_parms._fields[j]);
      Radx::fl32 missing = Fields::fieldMissingValue(_parms._fields[j]);
      
      for (int i=0; i<npt; ++i)
      {
	data[i] = missing;
      }
      ray->addField(name, units, npt, missing, data, true);
      wanted.push_back(name);
    }
    ray->trimToWantedFields(wanted);
    delete [] data;
  }
  return true;
#endif
}
