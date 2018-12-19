#include "AzGradientStateSpecialData.hh"
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <toolsa/LogStream.hh>
#include <cmath>
#include <algorithm>

//------------------------------------------------------------------
AzGradientStateSpecialData::AzGradientStateSpecialData(const RadxVol &vol)
{
  const vector<RadxSweep *> s = vol.getSweeps();
  for (size_t i=0; i<s.size(); ++i)
  {
    _setState(vol, *s[i]);
  }
}

//------------------------------------------------------------------
bool AzGradientStateSpecialData::getFloat(double &v) const
{
  return false;
}

//------------------------------------------------------------------
void AzGradientStateSpecialData::_setState(const RadxVol &vol,
					   const RadxSweep &s)
{
  double fixedAngle = s.getFixedAngleDeg();
  int sweepNumber = s.getSweepNumber();
  int r0 = s.getStartRayIndex();
  int r1 = s.getEndRayIndex();
  int n = s.getNRays();
  
  double angle0, angle1, da;
    
  // figure out delta azimuth
  da = _deltaAngleDegrees(vol, s, angle0, angle1);

  // check for full 360
  bool is360 = static_cast<int>(da*static_cast<double>(n)) == 360;
  if (is360)
  {
    const RadxRay *ray0=NULL, *ray1=NULL;

    // expect the last and first to be one da apart
    double daCross = fabs(angle1 - angle0);
    while (daCross >= 360)  daCross -=360;
    if (daCross != da)
    {
      LOG(WARNING) << "Unexpected da at crossover " << daCross
		   << " expect " << da;
    }
    // here is where we store pointers to the 0th and last azimuths in the sweep
    const vector<RadxRay *> rays = vol.getRays();
    ray0 = rays[r0];
    ray1 = rays[r1];
    _state.push_back(AzGradientState(fixedAngle, sweepNumber, r0, r1, ray0,
				     ray1));
    LOG(DEBUG_VERBOSE) << "sweep " << sweepNumber << " 360, fixedAngle:" 
		       << fixedAngle << "rayInd:[" << r0 << "," << r1 
		       << "], numRay=" << s.getNRays();
  }
  else
  {
    _state.push_back(AzGradientState(fixedAngle, sweepNumber));
    LOG(DEBUG_VERBOSE) << "sweep " << sweepNumber << " sector, fixedAngle:" 
		       << fixedAngle << "rayInd:[" << r0 << "," 
		       << r1 << "], numRay=" << s.getNRays();
  }
}

//------------------------------------------------------------------
double AzGradientStateSpecialData::_deltaAngleDegrees(const RadxVol &vol,
						      const RadxSweep &s,
						      double &angle0,
						      double &angle1) const
{
  const vector<RadxRay *> rays = vol.getRays();

  vector<double> angles;
  for (int i = s.getStartRayIndex(); 
       i <= static_cast<int>(s.getEndRayIndex()); ++i)
  {
    double a = rays[i]->getAzimuthDeg();
    angles.push_back(a);
  }

  vector<double> sangles(angles);
  sort(sangles.begin(), sangles.end());
  double da = 0;
  for (size_t ia=0; ia<sangles.size()-1; ++ia)
  {
    double dai = sangles[ia+1] - sangles[ia];
    if (ia == 0)
    {
      da = dai;
    }
    else
    {
      if (da != dai)
      {
	LOG(WARNING) << "Uneven angle spacing " << dai << " going with " << da;
      }
    }
  }

  angle0 = angles[0];
  angle1 = *(angles.rbegin());
  return da;
}    

  
