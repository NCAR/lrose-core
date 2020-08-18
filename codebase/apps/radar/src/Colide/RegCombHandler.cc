/**
 * @file RegCombHandler.cc
 *
 */

#include "RegCombHandler.hh"
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/LogStream.hh>
#include <cmath>

//-------------------------------------------------------------------------
RegCombHandler::RegCombHandler(double extension, double angleDiff, double minHit) :
  _extension(extension), _angleDiff(angleDiff), _minHit(minHit)
{
}
							
//-------------------------------------------------------------------------
RegCombHandler::~RegCombHandler(void)
{
}

    
//-------------------------------------------------------------------------
void RegCombHandler::process(const Grid2d * &reg, const Grid2d * &dir, Grid2d *out)
{
  out->dataCopy(*reg);
  GridAlgs alg(*out);
  bool change = true;

  while (change)
  {
    change = false;
    
    std::vector<double> regIndex = alg.listValues(1000);
    int n = (int)regIndex.size();
    for (int i=0; i<n-1; ++i)
    {
      for (int j=i+1; j<n; ++j)
      {
	if (_shouldCombine(alg, *dir, regIndex[i], regIndex[j]))
	{
	  LOG(DEBUG) << "Combining regions " << regIndex[i]
		     << " " << regIndex[j];
	  change = true;
	  // combine region indices i and j by renaming t he j'th one to i
	  alg.change(regIndex[j], regIndex[i]);

	  // decrease all remaining regions by 1
	  for (int k=j+1; k<n; ++k)
	  {
	    alg.change(regIndex[k], regIndex[k]-1.0);
	  }
	}
      }
    }
  }
  out->dataCopy(alg);
}


//------------------------------------------------------------------
bool RegCombHandler::_shouldCombine(const GridAlgs &data,
				    const Grid2d &orientation,
				    double r0, double r1) const
{
  // isolate region r0
  GridAlgs reg0(data);
  reg0.maskExcept(r0);
  
  // isolate region r1
  GridAlgs reg1(data);
  reg1.maskExcept(r1);
  
  // extend region r0, preserving orientation values
  GridAlgs extend(orientation);
  extend.maskMissingToMissing(reg0);
  extend.expandLaterally(_extension);

  // get overlap of extension with r1
  extend.maskMissingToMissing(reg1);

  // for each point in the overlap, compare the extend orientation with
  // the r1 orientation, which is the input grid
  int ngood = 0;
  for (int i=0; i<extend.getNdata(); ++i)
  {
    double v;
    if (extend.getValue(i, v))
    {
      double v2;
      if (orientation.getValue(i, v2))
      {
	double diff = fabs(v-v2);
	if (diff <= _angleDiff)
	{
	  ++ngood;
	}
      }
    }
  }
  return (ngood >= _minHit);
}



