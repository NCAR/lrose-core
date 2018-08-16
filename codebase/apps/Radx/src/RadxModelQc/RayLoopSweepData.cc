#include <RayLoopSweepData.hh>
#include <CircularLookupHandler.hh>
#include <radar/RadxApp.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
static double _variance(const std::vector<double> &vals, double num)
{
  double mean = 0;
  double count = 0;
  for (size_t i=0; i<vals.size(); ++i)
  {
    mean += vals[i];
    count += 1.0;
  }
  mean /= count;

  double var = 0.0;
  for (size_t i=0; i<vals.size(); ++i)
  {
    var += pow(vals[i] - mean, 2.0);
  }
  var /= count;
  return var;
}


RayLoopSweepData::RayLoopSweepData(void)
{
  _i0 = -1;
  _i1 = -1;
  _numDataPerRay = 0;
  _numRay = 0;
}

RayLoopSweepData::RayLoopSweepData(const std::string &name,
				   int i0, int i1,
				   const std::vector<RayxData> &rays):
  _i0(i0), _i1(i1), _name(name), _rays(rays)
{
  _numDataPerRay = 0;
  for (int i=i0; i<=i1; ++i)
  {
    int n = rays[i-i0].getNpoints();
    if (n > _numDataPerRay)
    {
      _numDataPerRay = n;
    }
  }
  _numRay = (int)rays.size();

}

RayLoopSweepData::~RayLoopSweepData(void)
{
}

int RayLoopSweepData::numData(void) const
{
  return _numDataPerRay*_numRay;
}

bool RayLoopSweepData::nameMatch(const std::string &n) const
{
  return n == _name;
}

bool RayLoopSweepData::getVal(int ipt, double &v) const
{
  return _rays[_rayIndex(ipt)].getV(_gateIndex(ipt), v);
}

void RayLoopSweepData::setVal(int ipt, double v)
{
  _rays[_rayIndex(ipt)].setV(_gateIndex(ipt), v);
}

void RayLoopSweepData::setMissing(int ipt)
{
  _rays[_rayIndex(ipt)].setV(_gateIndex(ipt), _rays[0].getMissing());
}

double RayLoopSweepData::getMissingValue(void) const
{
  return _rays[0].getMissing();
}

MathLoopData *RayLoopSweepData::clone(void) const
{
  RayLoopSweepData *ret = new RayLoopSweepData(*this);
  return (MathLoopData *)ret;
}

void RayLoopSweepData::setAllToValue(double v)
{
  for (int i=0; i<_numRay; ++i)
  {
    for (int j=0; j<_numDataPerRay; ++j)
    {
      _rays[i].setV(j, v);
    }
  }
}

void RayLoopSweepData::print(void) const
{
  printf("Sweep for %s [%d,%d]\n", _name.c_str(), _i0, _i1);
}

void RayLoopSweepData::modifyForOutput(const std::string &name,
				       const std::string &units)
{
  _name = name;
  for (int i=0; i<_numRay; ++i)
  {
    RadxApp::modifyRayForOutput(_rays[i], name, units, _rays[i].getMissing());
  }
}

int RayLoopSweepData::getNGates(void) const
{
  return _numDataPerRay;
}

void RayLoopSweepData::retrieveRayData(int rayIndex, float *data,
				       int nGatesPrimary) const
{
  _rays[rayIndex].retrieveSubsetData(data, nGatesPrimary);
}

std::string RayLoopSweepData::getUnits(void) const
{
  return _rays[0].getUnits();
}

int RayLoopSweepData::getNpoints(int rayIndex) const
{
  return _rays[rayIndex].getNpoints();
}

double RayLoopSweepData::getMissing(void) const
{
  return _rays[0].getMissing();
}

bool RayLoopSweepData::variance2d(const RayLoopSweepData &inp,
				  const CircularLookupHandler &lookup)
{
  // get az diff between first and last ray to see if a 360
  double a0 = _rays[_i0].getAzimuth();
  double a1 = _rays[_i1].getAzimuth();
  bool circular = false;
  if ((a0 > 355 && a1 < 5) || (a1 > 355 && a0 < 5))
  {
    circular = true;
  }
  else if ((a0 > 175 && a0 <= 180) && (a1 < -175 && a1 >= -180))
  {
    circular = true;
  }
  else if ((a1 > 175 && a1 <= 180) && (a0 < -175 && a0 >= -180))
  {
    circular = true;
  }
  else if (fabs(a1-a0) < 5)
  {
    circular = true;
  }

  // go through each ray and set the 2d var using nearby rays
  for (int i=0; i<_numRay; ++i)
  {
    // set output to all missing
    _rays[i].setAllToValue(_rays[i].getMissing());


    // // create an output ray for index i
    // RayxData out = _data[i];
    // RadxApp::modifyRayForOutput(out, outname, out.getUnits(),
    //  				out.getMissing());
    // // set all to missing
    // out.setAllToValue(out.getMissing());
    
    if (!_update2dVarOnRay(i, inp, lookup, circular))
    {
      LOG(WARNING) << "Failure for ray " << i;
    }
    // int nGatesPrimary = _rays[ii]->getNGates();
    // Radx::fl32 *data = new Radx::fl32[nGatesPrimary];
    // out.retrieveData(data, nGatesPrimary);
    // _rays[ii]->addField(outname, out.getUnits(), out.getNpoints(),
    // 			out.getMissing(), data, true);
    // vector<string> wanted;
    // wanted.push_back(name);
    // _rays[ii]->trimToWantedFields(wanted);
  // // now add in all the other ones
  // for (size_t i=1; i<_data.size(); ++i)
  // {
  //   _data[i].retrieveData(data, nGatesPrimary);
  //   _ray->addField(_data[i].getName(), _data[i].getUnits(),
  // 		   _data[i].getNpoints(),
  // 		   _data[i].getMissing(), data, true);
  // }
    // delete [] data;
  }
  return true;
}

//------------------------------------------------------------------
bool RayLoopSweepData::_update2dVarOnRay(int i,
					 const RayLoopSweepData &inp,
					 const CircularLookupHandler &lookup,
					 bool circular) 
{
  for (int r=0; r<lookup.nGates(); ++r)
  {
    if (r >= _rays[i].getNpoints())
    {
      break;
    }
    if (!_update2dVarGate(i, r, inp, lookup, circular))
    {
      // no big deal
    }
  }
  return true;
}

//------------------------------------------------------------------
 bool RayLoopSweepData::_update2dVarGate(int i, int r,
					 const RayLoopSweepData &inp,
					 const CircularLookupHandler &lookup,
					 bool circular)
{
  vector<double> data;
  double count = 0;
  const CircularLookup &l = lookup[r];
  for (int j=0; j<l.num(); ++j)
  {
    int rj = lookup[r].ithIndexR(j);
    int aj = lookup[r].ithIndexA(j);
    if (!_addLookupToData(i, r, rj, aj, inp, lookup, circular, data, count))
      return false;
  }
  if (count == 0)
  {
    // LOG(WARNING) << "No data";
    return true;
  }	
  else
  {
    double nd = (double)data.size();
    if (nd/count < 0.75)
    {
      //LOG(WARNING) << "NOt enough data";
      return true;
   }
  }
  
  // here is where output data is added at position r
  double v = _variance(data, count);
  _rays[i].setV(r, v);
  return true;
}

//------------------------------------------------------------------
bool RayLoopSweepData::_addLookupToData(int i, int r, int rj, int aj,
					const RayLoopSweepData &inp,
					const CircularLookupHandler &lookup,
					bool circular,
					vector<double> &data,
					double &count) const
{
  int ir = rj;
  if (ir < 0 || ir >= lookup.nGates())
  {
    return true;
  }
  int ia = i + aj;
  if (ia < 0)
  {
    if (circular)
    {
      ia = ia + _numRay;
    }
    else
    {
      return true;
    }
  }
  if (ia >= _numRay)
  {
    if (circular)
    {
      ia = ia - _numRay;
    }
    else
    {
      return true;
    }
  }

  // add data from ray ia, index ir to data
  if (ir < inp._rays[ia].getNpoints())
  {
    count ++;
    double v;
    if (inp._rays[ia].getV(ir, v))
    {
      data.push_back(v);
    }
  }
  return true;
}



