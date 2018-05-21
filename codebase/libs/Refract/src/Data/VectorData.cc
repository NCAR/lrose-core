/**
 * @file VectorData.cc
 */

#include <Refract/VectorData.hh>
#include <Refract/FieldWithData.hh>
#include <Refract/FieldDataPair.hh>
#include <Refract/RefractConstants.hh>
#include <Refract/RefractUtil.hh>
#include <toolsa/LogStream.hh>


void VectorData::setInitialQuality(int rMin, const VectorData &phaseDiffError,
				   const VectorIQ &iq, int numAz,
				   int numGates)
{
  for (int i=0; i<_num; ++i)
  {
    int r = FieldWithData::rIndex(i, numAz, numGates);
    if (r >= rMin)
    {
      if (phaseDiffError[i] != refract::INVALID)
      {
	_data[i] = iq[i].norm();
      }
    }
  }
}
	
double VectorData::sumSquares(int numSum, int centerIndex) const
{
  int maxOff = numSum/2;  // truncates, 3 gives 1, 5 give 2, and so on.

  double ret = 0.0;
  for (int i=centerIndex-maxOff; i<=centerIndex + maxOff; ++i)
  {
    ret += RefractUtil::SQR(_data[i]);
  }
  return ret;
}
  
