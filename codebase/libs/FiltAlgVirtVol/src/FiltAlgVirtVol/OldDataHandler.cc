#include <FiltAlgVirtVol/OldDataHandler.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/LogStream.hh>
#include <euclid/GridAlgs.hh>
#include <vector>

//----------------------------------------------------------------------------
void OldDataField::initialize(const std::string &url,
			      const std::string &fieldName,
			      const std::vector<time_t> &times,
			      const VirtVolParms &parms)
{
  for (size_t j=0; j<times.size(); ++j)
  {
    OldDataAtTime ot(times[j]);
    ot.initialize(url, times[j], fieldName, parms); 
    _data.push_back(ot);
  }
}
  
//----------------------------------------------------------------------------
void OldDataAtTime::initialize(const std::string &url, const time_t &t,
			       const std::string &fieldName,
			       const VirtVolParms &parms)
{
  DsMdvx mdv;
  mdv.setReadTime(Mdvx::READ_FIRST_BEFORE, url, 0, t);
  mdv.addReadField(fieldName);
  if (parms.restrict_vertical_levels)
  {
    mdv.setReadVlevelLimits(parms._vertical_level_range[0],
			    parms._vertical_level_range[1]);
  }
  LOG(DEBUG) << "Reading";
  if (mdv.readVolume())
  {
    LOG(WARNING)<< "No historical data for " << url;
    return;
  }

  LOG(DEBUG) << "Storing OLD locally " << fieldName;
  MdvxField *f = mdv.getFieldByName(fieldName);
  if (f == NULL)
  {
    LOG(ERROR) << "reading field " << fieldName << " from " << url;
    return;
  }
  f->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  Mdvx::field_header_t hdr = f->getFieldHeader();

  // test not done here
  // if (_nz != hdr.nz || _nx != hdr.nx || _ny != hdr.ny)
  // {
  //   LOG(ERROR) << "Unequal dimensions";
  //   break;
  // }

  for (int j=0; j<hdr.nz; ++j)
  {
    Grid2d g(fieldName, hdr.nx, hdr.ny, hdr.missing_data_value);
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
    _dataAtHeight.push_back(gd);
  }
}

//----------------------------------------------------------------------------
OldDataSweepField::OldDataSweepField(const OldDataField &o, int zIndex)
{
  _zIndex = zIndex;
  _fieldName = o.fieldName();
  for (size_t i=0; i<o.size(); ++i)
  {
    _data.push_back(OldDataAtTimeHeight(o[i], zIndex));
  }
  _vectorIsSet = _data.size() > 0;
}

//----------------------------------------------------------------------------
OldDataAtTimeHeight::OldDataAtTimeHeight(const OldDataAtTime &o, int zIndex) :
  _time(o.getTime())
{
  if (o.hasGrid(zIndex))
  {
    _hasData = o.grid2d(zIndex, _dataAtHeight);
  }
  else
  {
    _hasData = false;
    _dataAtHeight = GriddedData();
  }    
}

//----------------------------------------------------------------------------
bool OldDataSweepField::constructWeightedFields(const time_t &t,
						const FuzzyF &dtToWt,
						std::vector<Grid2d> &inps,
						std::vector<double> &weights) const
{
  bool status = true;
  inps.clear();
  weights.clear();
  for (size_t j=0; j<_data.size(); ++j)
  {
    if (_data[j].hasData())
    {
      time_t tOld = _data[j].getTime();
      weights.push_back(dtToWt.apply(t - tOld));
      Grid2d g;
      if (_data[j].getData(g))
      {
	inps.push_back(g);
      }
      else
      {
	LOG(ERROR) << "inconsistent data access " << j;
	status = false;
      }	
    }
    else
    {
      LOG(WARNING) << "Missing data for height " << j;
      status = false;
    }
  }
  return status;
}

  
//----------------------------------------------------------------------------
bool OldDataSweepField::constructAgeWithMax(const time_t &t, Grid2d &ageGrid) const
{
  bool first = true;
  GridAlgs maxGrid;
  GridAlgs age;

  if (!_vectorIsSet)
  {
    return false;
  }

  // expect one data grid per old time
  for (size_t j=0; j<_data.size(); ++j)
  {
    if (!_data[j].hasData())
    {
      LOG(ERROR) << "Missing data at " << j;
      return false;
    }
    Grid2d g;
    _data[j].getData(g);
    time_t tOld = _data[j].getTime();
    double minutesOld = (double)(t-tOld)/60.0;
    if (first)
    {
      first = false;
      maxGrid = g;
      age = g;
      age.setAllToValue(minutesOld);
      age.maskMissingToMissing(g);
    }
    else
    {
      for (int i=0; i<g.getNdata(); ++i)
      {
	double mv;
	if (maxGrid.getValue(i, mv))
	{
	  double v;
	  if (g.getValue(i, v))
	  {
	    if (v > mv)
	    {
	      maxGrid.setValue(i, v);
	      age.setValue(i, minutesOld);
	    }
	  }
	}
	else
	{
	  double v;
	  if (g.getValue(i, v))
	  {
	    maxGrid.setValue(i, v);
	    age.setValue(i, minutesOld);
	  }
	}
      }
    }
  }	  
  ageGrid = age;
  return true;
}

//----------------------------------------------------------------------------
void OldData::addField(const time_t &triggerTime,
		       const std::string &fieldName,
		       const std::string &url,
		       const int maxSecondsBack,
		       const VirtVolParms &parms)
{
  DsMdvx D;
  D.setTimeListModeValid(url, triggerTime-maxSecondsBack, triggerTime-1);
  D.compileTimeList();
  vector<time_t> times = D.getTimeList();

  OldDataField oldField(fieldName);
  oldField.initialize(url, fieldName, times, parms);
  _fields.push_back(oldField);
}

//----------------------------------------------------------------------------
OldSweepData::OldSweepData(const OldData &o, int zIndex)
{
  for (size_t i=0; i<o.size();++i)
  {
    _data.push_back(OldDataSweepField(o[i], zIndex));
  }
}
