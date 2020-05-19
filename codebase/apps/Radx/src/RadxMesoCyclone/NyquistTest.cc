#include "NyquistTest.hh"
#include "Sweep.hh"
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//------------------------------------------------------------------
void NyquistTest::apply(const Grid2d &data, const Grid2d &mask,
			const Sweep &v, Grid2d &out)
{
  bool circular = v.isCircular();
  out.setAllMissing();

  // go through each
  for (int i=0; i<out.getNy(); ++i)
  {
    if (!_updateOneRay(i, data, mask, circular, out))
    {
      LOG(WARNING) << "Failure for ray " << i;
    }
  }
  return;
}

//------------------------------------------------------------------
bool NyquistTest::_updateOneRay(int i, const Grid2d &data,
				const Grid2d &mask,
				bool circular, Grid2d &out)
{
  for (int r=0; r<_t->nGates(); ++r)
  {
    if (!data.isMissing(r, i) && !mask.isMissing(r, i))
    {
      if (!_updateGate(i, r, data, circular, out))
      {
	// no big deal
      }
    }
  }
  return true;
}

//------------------------------------------------------------------
bool NyquistTest::_updateGate(int i, int r,
				const Grid2d &data,
				bool circular, Grid2d &out)
{
  vector<double> vdata1, vdata2;
  double count1 = 0;
  double count2 = 0;
  const TemplateLookup &l = (*_t)[r];
  for (int j=0; j<l.num1(); ++j)
  {
    int rj = l.ithIndex1R(j);
    int aj = l.ithIndex1A(j);
    if (!_addLookupToData(i, r, rj, aj, data, circular, vdata1, count1))
      return false;
  }

  for (int j=0; j<l.num2(); ++j)
  {
    int rj = l.ithIndex2R(j);
    int aj = l.ithIndex2A(j);
    if (!_addLookupToData(i, r, rj, aj, data, circular, vdata2, count2))
      return false;
  }
  if (count1 == 0 || count2 == 0)
  {
    // LOG(WARNING) << "No data";
    return true;
  }	
  else
  {
    double nd = (double)vdata1.size();
    if (nd/count1 < _parms.min_percent_good)
    {
      //LOG(WARNING) << "NOt enough data";
      return true;
   }
    nd = (double)vdata2.size();
    if (nd/count2 < _parms.min_percent_good)
    {
      //LOG(WARNING) << "NOt enough data";
      return true;
   }
  }
  
  // here is where output data is added at position r
  double v = _process(vdata1, vdata2, count1, count2);
  out.setValue(r, i, v);
  return true;
}

//------------------------------------------------------------------
bool NyquistTest::_addLookupToData(int i, int r, int rj, int aj,
				    const Grid2d &data,
				    bool circular,
				    vector<double> &vdata,
				    double &count) const
{
  int ir = rj;
  if (ir < 0 || ir >= _t->nGates())
  {
    return true;
  }
  int ny = data.getNy();
  int ia = i + aj;
  while  (ia < 0)
  {
    if (circular)
    {
      ia = ia + ny;
    }
    else
    {
      return true;
    }
  }
  while (ia >= ny)
  {
    if (circular)
    {
      ia = ia - ny;
    }
    else
    {
      return true;
    }
  }

  // add data from ray ia, index ir to data
  if (ir < data.getNx())
  {
    double v;
    if (data.getValue(ir, ia, v))
    {
      count ++;
      v = _parms._nyquistFuzzy.apply(fabs(v));
      vdata.push_back(v);
    }
  }
  return true;
}

double NyquistTest::_process(const std::vector<double> &data1,
			      const std::vector<double> &data2,
			      double count1, double count2)
{
  // looking for data1 mostly postive or mostly negative, and data2 the opposite

  double c = 0;
  double s = 0;
  for (size_t i=0; i<data1.size(); ++i)
  {
    c ++;
    s += data1[i];
  }
  for (size_t i=0; i<data2.size(); ++i)
  {
    c ++;
    s += data2[i];
  }

  if (c > 0)
  {
    double ave = s/c;
    return ave;
  }
  else
  {
    return 0.0;
  }
}
