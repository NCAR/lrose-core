/**
 * @file VirtVolVolume.cc
 */

//------------------------------------------------------------------
#include <FiltAlgVirtVol/VirtVolVolume.hh>
#include <FiltAlgVirtVol/VirtVolSweep.hh>
#include <FiltAlgVirtVol/PolarCircularTemplate.hh>
#include <FiltAlgVirtVol/PolarCircularFilter.hh>
#include <FiltAlgVirtVol/VertData2d.hh>
#include <FiltAlgVirtVol/VirtVolFuzzy.hh>
#include <FiltAlgVirtVol/ShapePolygons.hh>
#include <FiltAlgVirtVol/VolumeTime.hh>
#include <rapmath/UnaryNode.hh>
#include <rapmath/FunctionDef.hh>
#include <dsdata/DsUrlTrigger.hh>

#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>

#include <toolsa/LogMsgStreamInit.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <cstdlib>

const std::string VirtVolVolume::_parmsFuzzyStr = "ParmsFuzzy";
const std::string VirtVolVolume::_parmsCircularTemplateStr = "ParmsCircularTemplate";
const std::string VirtVolVolume::_verticalConsistencyStr = "VerticalConsistency";
const std::string VirtVolVolume::_verticalClumpFiltStr = "VerticalDataClumpFilt";
const std::string VirtVolVolume::_shapeFiltStr = "ComputeShapes";
const std::string VirtVolVolume::_shapeFixedFiltStr = "ComputeFixedSizeShapes";
const std::string VirtVolVolume::_volumeTimeStr = "VolumeTime";

					      
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
std::vector<FunctionDef> VirtVolVolume::virtVolUserUnaryOperators(void)
{
  std::vector<FunctionDef> ret;
  ret.push_back(FunctionDef(_parmsFuzzyStr, "P", "x0, y0, x1, y1, ...",
			    "pairs of points making up a fuzzy function"));
  ret.push_back(FunctionDef(_parmsCircularTemplateStr, "M", "r, minr",
			    "This should be a volume_before filter, r=radius Km of the template, minr = min radius Km to create a template"));
  ret.push_back(FunctionDef(_verticalConsistencyStr, "M", "data, isNyquist, template",
			    "This should be a volume_after filter, template is a circular template.  Data is maximized using this template at each point, then this filter looks at values at each heights and counts heights with data vs. without to get percent with data, then multiplies by average to get final output.  Currently nyquist input is not used"));
  ret.push_back(FunctionDef(_verticalClumpFiltStr, "M", "data, threshold, minpct",
			    "Take 2d vertical consistency, clump, and keep only clumps with minpct of the points >= threshold"));
  ret.push_back(FunctionDef(_shapeFiltStr, "M", "data, mode",
			    "Take 2d data, clump, and generate polygons around each clump, which are returned as special data. mode=0 means diamonds the size of the region, model=1 means shapes that hug the clumps"));
  ret.push_back(FunctionDef(_shapeFixedFiltStr, "M", "data, sizeKm",
			    "Take 2d data, clump, and generate polygons around each clump, which are returned as special data.  The shapes are fixed size diamonds of sizeKm on  side"));
  ret.push_back(FunctionDef(_volumeTimeStr, "T", "",
			    "volume_before filter, returns volume time as a user data type"));
  return ret;
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
  for (size_t i=0; i<_parms->_inputUrl.size(); ++i)
  {
    if (_initialInitializeInput(_time, _parms->_inputUrl[i]))
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
  for (size_t i=0; i<_parms->_inputUrl.size(); ++i)
  {
    if (!_initializeInput(_time, _parms->_inputUrl[i]))
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
    if (_parms->hasOutputField(name))
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
  if (_parms->hasOutputField(name))
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
MathUserData *VirtVolVolume::processVirtVolUserVolumeFunction(const UnaryNode &p)
{
    // pull out the keyword
  string keyword;
  if (!p.getUserUnaryKeyword(keyword))
  {
    return NULL;
  }
  vector<string> args = p.getUnaryNodeArgStrings();
  if (keyword == _parmsFuzzyStr)
  {
    return _computeFuzzyFunction(args);
  }
  else if (keyword == _parmsCircularTemplateStr)
  {
    // expect 2 double arguments
    if (args.size() != 2)
    {
      LOG(ERROR) << "Wrong number of args want 2 got " << args.size();
      return NULL;
    }
    return _computeParmsCircularTemplate(args[0], args[1]);
  }
  else if (keyword == _verticalConsistencyStr)
  {
    if (args.size() != 3)
    {
      LOG(ERROR) << "Wrong number of args want 3 got " << args.size();
      return NULL;
    }
    return _computeVerticalConsistency(args[0], args[1], args[2]);
  }
  else if (keyword == _verticalClumpFiltStr)
  {
    if (args.size() != 3)
    {
      LOG(ERROR) << "Wrong number of args want 3 got " << args.size();
      return NULL;
    }
    return _computeVerticalClumpFilt(args[0], args[1], args[2]);
  }
  else if (keyword == _shapeFiltStr)
  {
    if (args.size() != 2)
    {
      LOG(ERROR) << "Wrong number of args want 1 got " << args.size();
      return NULL;
    }
    return _computeShapes(args[0], args[1]);
  }
  else if (keyword == _shapeFixedFiltStr)
  {
    if (args.size() != 2)
    {
      LOG(ERROR) << "Wrong number of args want 1 got " << args.size();
      return NULL;
    }
    return _computeFixedShapes(args[0], args[1]);
  }
  else if (keyword == _volumeTimeStr)
  {
    // so simple put it here
    VolumeTime *vt = new VolumeTime(_time);
    return (MathUserData *)vt;
  }
  else
  {
    return NULL;
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
void VirtVolVolume::specialOutput2d(const time_t &t,
				    const UrlParms &u)
{
  if (u.url_type != UrlParams::VOLUME)
  {
    LOG(DEBUG) << "output of non-volume data assumed done by app - "
	       << u.url;
    return;
  }
  if (u.data_type != UrlParams::GRID)
  {
    LOG(DEBUG) << "output of non-gridded data assumed done by app - "
	       << u.url;
    return;
  }

  vector<string> names = u.getNames();
  vector<Grid2d> grids;
  for (size_t i=0; i<names.size(); ++i)
  {
    std::string name = names[i];
    MathUserData *u = _special->matchingDataPtr(name);
    if (u != NULL)
    {
      // this is the VertData..probably should check somehow.
      VertData2d *v = (VertData2d *)u;
      Grid2d g(v->constGridRef());
      g.setName(names[i]);
      grids.push_back(g);
    }
  }	

  _mdv.output2d(t, grids, u);
}

//------------------------------------------------------------------
void VirtVolVolume::output(const time_t &t)
{
  // for each output url
  for (size_t i=0; i<_parms->_outputUrl.size(); ++i)
  {
    _outputToUrl(t, _parms->_outputUrl[i]);
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
  if (userKey == _parmsFuzzyStr)
  {
    return true; // there are actually no args
  }
  if (userKey == _parmsCircularTemplateStr)
  {
    return true; // there are actually no args
  }
  else if (userKey == _verticalConsistencyStr)
  {
    if (names.size() != 3)
    {
      LOG(ERROR) << "Expect 3 inputs got " << names.size();
      return false;
    }
    return true;
  }
  else if (userKey == _verticalClumpFiltStr)
  {
    // no args. What is happening here?  Need to debug.
    return true;
  }
  else if (userKey == _shapeFiltStr)
  {
    return true;
  }
  else if (userKey == _shapeFixedFiltStr)
  {
    return true;
  }
  else if (userKey == _volumeTimeStr)
  {
    return true;
  }
  else
  {
    return virtVolSynchUserInputs(userKey, names);
  }

}

//------------------------------------------------------------------
bool VirtVolVolume::isCircular(void) const
{
  Mdvx::coord_t coord = proj().getCoord();
  return fabs(coord.ny*coord.dy >= 358);
}

//------------------------------------------------------------------
int VirtVolVolume::getNGates(void) const
{
  return _mdv.getNGates();
}  

//------------------------------------------------------------------
double VirtVolVolume::getDeltaGateKm(void) const
{
  return _mdv.getDeltaGateKm();
}

//------------------------------------------------------------------
double VirtVolVolume::getDeltaAzDeg(void) const
{
  return _mdv.getDeltaAzDeg();
}

//------------------------------------------------------------------
double VirtVolVolume::getStartRangeKm(void) const
{
  return _mdv.getStartRangeKm();
}

//------------------------------------------------------------------
int VirtVolVolume::nz(void) const
{
  return _mdv.nz();
}

//------------------------------------------------------------------
bool
VirtVolVolume::_initialInitializeInput(const time_t &t, const UrlParms &u)
{
  if (u.url_type != UrlParams::VOLUME)
  {
    return false;
  }
  vector<string> fields = u.getNames();
  if (fields.empty())
  {
    return false;
  }

  if (_mdv.initialize(t, u, fields[0], _parms))
  {
    _data.clear();
    for (int i=0; i<_mdv.nz(); ++i)
    {
      _data.push_back(GridFields(i));
    }
    return true;
  }
  return false;
}

//------------------------------------------------------------------
bool VirtVolVolume::_initializeInput(const time_t &t, const UrlParms &u)
{
  if (u.url_type != UrlParams::VOLUME)
  {
    LOG(DEBUG) << "Initializion of " << u.url << " not needed";
    return true;
  }
    
  vector<string> fields = u.getNames();
  if (!fields.empty())
  {
    if (!_mdv.readAllFields(t, u.url, fields, _parms))
    {
      return false;
    }


    for (size_t i=0; i<fields.size(); ++i)
    {
      if (!_initializeInputField(fields[i], u.url))
      {
	return false;
      }
    }
  }
  return true;
}

//-----------------------------------------------------------------
bool VirtVolVolume::_initializeInputField(const std::string &field,
					  const std::string &url)
{
  LOG(DEBUG) << "Storing locally " << field;
  vector<GriddedData> gd = _mdv.initializeInputField(field, url, _parms);
  if (gd.empty())
  {
    return false;
  }

  for (size_t j=0; j<gd.size(); ++j)
  {
    _data[j]._grid2d.push_back(gd[j]);
  }
  return true;
}

//------------------------------------------------------------------
void VirtVolVolume::_outputToUrl(const time_t &t, const UrlParms &u)
{
  if (u.url_type != UrlParams::VOLUME)
  {
    LOG(DEBUG) << "output of non-volume data assumed done by app - "
	       << u.url;
    return;
  }

  vector<string> names = u.getNames();
  if (names.empty())
  {
    LOG(WARNING) << "Nothing to write to url " << u.url;
    return;
  }
  _mdv.initializeOutput(t, 3);

  for (size_t i=0; i<names.size(); ++i)
  {
    double missing;
    if (!_missingValueForField(names[i], missing))
    {
      return;
    }

    vector<Grid2d> data3d = _getData3d(names[i]);
    if (data3d.empty())
    {
      return;
    }
    _mdv.addOutputField(t, names[i], data3d, missing);
      
    // for (size_t i=0; i<names.size(); ++i)
    // {
    //   _outputFieldToUrl(names[i], t, out);
    // }
  }
  _mdv.write(u);
}

// //------------------------------------------------------------------
// void VirtVolVolume::_outputFieldToUrl(const std::string &name,
// 				      const time_t &t, DsMdvx &out)

//   Mdvx::field_header_t fieldHdr = _tweakedFieldHdr(3, _nx, _ny, _nz, t, 
// 						   name,  missing);
//   // finally, add the field
//   MdvxField *f = new MdvxField(fieldHdr, _vlevelHdr, (void *)0,true,false);
//   fl32 *fo = (fl32 *)f->getVol();

//   // Loop through again and populate the new field
//   for ( size_t z=0; z<_data.size(); ++z)
//   {
//     for (size_t f=0; f<_data[z]._grid2d.size(); ++f)
//     {
//       if (_data[z]._grid2d[f].getName() == name)
//       {
// 	for (int i=0; i<_nx*_ny; ++i)
// 	{
// 	  fo[i+z*_nx*_ny] = _data[z]._grid2d[f].getValue(i);
// 	}
// 	break;
//       }
//     }
//   }
//   f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
//   out.addField(f);
// }

//------------------------------------------------------------------
MathUserData *VirtVolVolume::
_computeParmsCircularTemplate(const std::string &rstr,
			      const std::string &minrstr)
{
  double r, minr;
  if (sscanf(rstr.c_str(), "%lf", &r) != 1)
  {
    LOG(ERROR) << "Cannot scan " << rstr << " as a double";
    return NULL;
  }
  if (sscanf(minrstr.c_str(), "%lf", &minr) != 1)
  {
    LOG(ERROR) << "Cannot scan " << minrstr << " as a double";
    return NULL;
  }

  PolarCircularTemplate *t = new PolarCircularTemplate(r, proj(), minr);
  return (MathUserData *)t;
}

//------------------------------------------------------------------
MathUserData *VirtVolVolume::
_computeFuzzyFunction(const std::vector<std::string> &args)
{
  int nargs = (int)args.size();
  if (nargs % 2 != 0)
  {
    LOG(ERROR) << "Need an even number or args";
    return NULL;
  }
  std::vector<pair<double,double> > xy;
  for (size_t i=0; i<args.size(); i+=2)
  {
    double x, y;
    if (sscanf(args[i].c_str(), "%lf", &x) != 1)
    {
      LOG(ERROR) << "Cannot scan " << args[i] << " as a double";
      return NULL;
    }
    if (sscanf(args[i+1].c_str(), "%lf", &y) != 1)
    {
      LOG(ERROR) << "Cannot scan " << args[i+1] << " as a double";
      return NULL;
    }
    xy.push_back(pair<double,double>(x, y));
  }

  VirtVolFuzzy *v = new VirtVolFuzzy(xy);
  if (v->ok())
  {
    return (MathUserData *)v;
  }
  else
  {
    delete v;
    return NULL;
  }
}

//------------------------------------------------------------------
MathUserData *VirtVolVolume::
_computeVerticalConsistency(const std::string &dataName,
			    const std::string &isNyquistName,
			    const std::string &templateName)
{
  std::vector<GriddedData> data = getField3d(dataName);
  if (data.empty())
  {
    LOG(ERROR) << "No data " << dataName;
    return NULL;
  }

  std::vector<GriddedData> isNyquist = getField3d(isNyquistName);
  if (isNyquist.empty())
  {
    LOG(ERROR) << "No data " << isNyquistName;
    return NULL;
  }

  MathUserData *udata = _special->matchingDataPtr(templateName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No user data " << templateName;
    return NULL;
  }
  else
  {
    const PolarCircularTemplate *pt = (const PolarCircularTemplate *)udata;
    return _consistency(data, isNyquist, *pt);
  }
}

//------------------------------------------------------------------
MathUserData *VirtVolVolume::
_computeVerticalClumpFilt(const std::string &dataName,
			  const std::string &threshName,
			  const std::string &pctName)
{
  MathUserData *udata = _special->matchingDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data " << dataName;
    return NULL;
  }
  VertData2d *vdata = (VertData2d *)udata;
  double thresh, pct;
  if (sscanf(threshName.c_str(), "%lf", &thresh) != 1)
  {
    LOG(ERROR) << "Scanning " << threshName << " as double";
    return NULL;
  }
  if (sscanf(pctName.c_str(), "%lf", &pct) != 1)
  {
    LOG(ERROR) << "Scanning " << pctName << " as double";
    return NULL;
  }

  VertData2d *clumped = vdata->clump(thresh, pct);
  return (MathUserData *)clumped;
}

//------------------------------------------------------------------
MathUserData *VirtVolVolume::_computeShapes(const std::string &dataName,
					    const std::string &modeName)
{
  MathUserData *udata = _special->matchingDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data " << dataName;
    return NULL;
  }
  int mode;
  if (sscanf(modeName.c_str(), "%d", &mode) != 1)
  {
    LOG(ERROR) << "scanning " << modeName << " as an int";
    return NULL;
  }
  VertData2d *vdata = (VertData2d *)udata;
  int expireSeconds = 100;
  bool isDiamond;
  if (mode == 0)
  {
    isDiamond = true;
  }
  else if (mode == 1)
  {
    isDiamond = false;
  }
  else
  {
    LOG(ERROR) << "Mode must be 0 or 1";
    return NULL;
  }
  
  ShapePolygons *s = new ShapePolygons(_time, expireSeconds, proj(),
				       vdata->constGridRef(), isDiamond);
  return (MathUserData *)s;
}

//------------------------------------------------------------------
MathUserData *VirtVolVolume::_computeFixedShapes(const std::string &dataName,
						 const std::string &sizeName)
{
  MathUserData *udata = _special->matchingDataPtr(dataName);
  if (udata == NULL)
  {
    LOG(ERROR) << "No data " << dataName;
    return NULL;
  }
  double sizeKm;
  if (sscanf(sizeName.c_str(), "%lf", &sizeKm) != 1)
  {
    LOG(ERROR) << "scanning " << sizeName << " as an int";
    return NULL;
  }
  VertData2d *vdata = (VertData2d *)udata;
  int expireSeconds = 100;
  
  ShapePolygons *s = new ShapePolygons(_time, expireSeconds, proj(),
				       sizeKm, vdata->constGridRef());
  return (MathUserData *)s;
}

//------------------------------------------------------------------
MathUserData *VirtVolVolume::_consistency(const std::vector<GriddedData> &data,
					  const std::vector<GriddedData> &isNyquist,
					  const PolarCircularTemplate &pt)
{
  Grid2d out(data[0]);
  out.setAllMissing();

  vector<Grid2d> dilated;

  // pull out every grid and expand it
  for (size_t i=0; i<data.size(); ++i)
  {
    Grid2d a(data[i]);

    PolarCircularFilter::dilate(a, pt);
    dilated.push_back(a);
  }
  for (int i=0; i<out.getNdata(); ++i)
  {
    double ngood = 0, nbad = 0, min = 0, max = 0;
    bool first = true;
    for (size_t j=0; j< dilated.size(); ++j)
    {
      double v;
      if (dilated[j].getValue(i, v))
      {
	if (first)
	{
	  first = false;
	  min = max = v;
	}
	else
	{
	  if (v < min) min = v;
	  if (v > max) max = v;
	}
	++ngood;
      }
      else
      {
	++nbad;
      }
    }
    if (ngood > 0)
    {
      // simple score
      double v = ngood/(ngood+nbad);
      v = v*(max+min)/2.0;
      // later use min and max, and do something fancier
      out.setValue(i, v);
    }
  }
  
  VertData2d *ret = new VertData2d(out, 0.0);
  return (MathUserData *)ret;
}

//------------------------------------------------------------------
 bool VirtVolVolume::_missingValueForField(const std::string &name, double &missingValue) const
 {

  // Try to get some sample data so as to set the missing values
  bool got = false;
  for ( size_t z=0; z<_data.size(); ++z)
  {
    for (size_t f=0; f<_data[z]._grid2d.size(); ++f)
    {
      if (_data[z]._grid2d[f].getName() == name)
      {
	got = true;
	missingValue = _data[z]._grid2d[f].getMissing();
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
    LOG(ERROR) << "Missing data " << name;
    LOG(WARNING) << "Write some code to fill missing";
    return false;
  }
  else
  {
    return true;
  }
 }


//------------------------------------------------------------------
 std::vector<Grid2d> VirtVolVolume::_getData3d(const std::string &name) const
{
  vector<Grid2d> ret;
  
  // Loop through again and populate the new field
  for ( size_t z=0; z<_data.size(); ++z)
  {
    bool ok = false;
    for (size_t f=0; f<_data[z]._grid2d.size(); ++f)
    {
      if (_data[z]._grid2d[f].getName() == name)
      {
	ret.push_back(_data[z]._grid2d[f]);
	ok = true;
	break;
      }
    }
    if (!ok)
    {
      ret.clear();
      LOG(ERROR) << "Didn't find name at all heights " << name;
    }
  }
  return ret;
}
