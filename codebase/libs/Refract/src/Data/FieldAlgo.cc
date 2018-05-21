/**
 * @file FieldAlgo.cc
 */
#include <Refract/FieldAlgo.hh>
#include <Refract/FieldDataPair.hh>
#include <Refract/MdvCreate.hh>
#include <Refract/FieldWithData.hh>
#include <Refract/RefractInput.hh>
#include <toolsa/LogStream.hh>
#include <string.h>

//---------------------------------------------------------------------------
FieldAlgo::FieldAlgo(const FieldWithData &F) : FieldWithData(F)
{
}

//---------------------------------------------------------------------------
FieldAlgo::FieldAlgo(const MdvxField *example, const std::string &name,
		     const std::string &units, double value) :
  FieldWithData(example, name, units, value)
{

}

//---------------------------------------------------------------------------
FieldAlgo::~FieldAlgo(void)
{
  // _free();
}
  
//---------------------------------------------------------------------------
void FieldAlgo::normalize(const FieldAlgo &meanSnrField, const int *pixCount)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (pixCount[i] == 0)
    {
      // don't want to divide by zero
      _data[i] = RefractInput::INVALID;
    }
    else
    {
      _data[i] = meanSnrField._data[i]/static_cast<double>(pixCount[i]);
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::normalize(const int *norm)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (norm[i] != 0)
    {
      _data[i] =_data[i]/static_cast<fl32>(norm[i]);
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::incrementNorm(const FieldDataPair &data)
{
  for (int i=0; i<_scanSize; ++i)
  {
    _data[i] += data[i].norm();
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::setStrength(const FieldAlgo &meanSnr, const int *pixelCount)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (pixelCount[i] == 0)
    {
      _data[i] = RefractInput::INVALID;
    }
    else
    {
      _data[i] = meanSnr[i]/static_cast<double>(pixelCount[i]);
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::setNcp(const FieldAlgo &strength, const FieldDataPair &sumAB,
		       const FieldAlgo &sumP, const double *fluctSnr,
		       const int *pixelCount, int r_min)
{
  int i = 0;
  for (int a=0; a<_numAzimuth; ++a)
  {
    for (int r=0; r<_numGates; ++r, ++i)
    {
      double fpix = pixelCount[i];
      if (sumP[i] == 0.0 || pixelCount[i] == 0)
      {
	continue;
      }
      double ncp = 0.0;
      if (r < r_min)
      {
	ncp = 0.5;
      }
      else
      {
	ncp = sumAB[i].normSquared()/sumP[i]/fpix;
	// ncp = sumAB.constData().normSquared(i)/sumP[i]/fpix;
	if (fpix > 1.0)
	{
	  double sfpix = sqrt(fpix);
	  ncp = (ncp - 1.0/sfpix) / (1.0 - 1.0/sfpix);
	  if (ncp < 0.001) ncp = 0.001;
	  if (ncp > 0.999) ncp = 0.999;
	}
      }
      ncp *= 1.0/(1.0 + pow(10.0, -0.1*strength[i]));

      double strength_correction = exp(-0.001*pow(fluctSnr[i], 4.0))/
	exp(-0.002);
      if (strength_correction > 1.0)
      {
	strength_correction = 1.0;
      }
      if (strength_correction < 0.1)
      {
	strength_correction = 0.1;
      }
      ncp *= strength_correction;
      if (ncp < 0)
      {
	ncp = 0.0;
      }
      _data[i] = ncp;
    }
  }
}

//----------------------------------------------------------------------------
void FieldAlgo::sideLobeFilter(const FieldAlgo &strength, int r_min,
			       double contamin_pow)
{

  // First, look for mainlobe contamination on nearby azimuths
  int index=0;
  int az = 0;
  for (az = 0, index = 0; az < _numAzimuth; ++az)
  {
    int r;
    for (r = r_min, index += r_min; r < _numGates; ++r, ++index)
    {
      if (_data[index] > 0.0)
      {
	// Get the power at the current gate

	double local_pow = pow(10.0, 0.1 * strength[index]);

	// Get the power at the gates in the surrounding beams

	int prev_az = (az + _numAzimuth- 1) % _numAzimuth;
	double near_pow = pow(10.0, 0.1*strength[prev_az * _numGates + r]);

	int next_az = (az + 1) % _numAzimuth;
	near_pow += pow(10.0, 0.1 * strength[next_az * _numGates + r]) ;

	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  double side_correction =
	    exp(-0.5 * RefractInput::SQR((near_pow - 2.5 * contamin_pow) /
				      (2.5 * contamin_pow)));
	  if (side_correction < 0.1)
	  {
	    side_correction = 0.1;
	  }
	  _data[index]  *= side_correction;
	}
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::rangeSidelobeFilter(const FieldAlgo &strength,
				    int r_min, double contamin_pow)
{
  int index;
  int az;

  for (az = 0, index = 0; az < _numAzimuth; ++az)
  {
    int r;
    for (r = r_min, index += r_min; r < _numGates; ++r, ++index)
    {
      if (_data[index] > 0.0)
      {
	// Get the power at the current gate

	float local_pow = pow(10.0, 0.1 * strength[index]);

	// Get the power at the surrounding gates along this beam

	float near_pow;
	
	if (r == 0)
	{
	  near_pow = RefractInput::VERY_LARGE;
	}
	else
	{
	  near_pow = pow(10.0, 0.1 * strength[index-1]);
	}
	
	if (r < _numGates - 1)
	{
	  near_pow += pow(10.0, 0.1 * strength[index+1]);
	}
	
	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  float side_correction =
	    exp(-0.5 * RefractInput::SQR((near_pow - 2.5 * contamin_pow) /
				      (2.5 * contamin_pow)));
	  if (side_correction < 0.1)
	  {
	    side_correction = 0.1;
	  }
	  _data[index] *= side_correction;
	}
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::sidelobe360Filter(const FieldAlgo &strength,
				  const FieldAlgo &snr, int r_min,
				  double side_lobe_pow)
{
  int index;
  for (int r = r_min; r < _numGates; ++r)
  {
    // Sum the power values at all gates the same distance as this gate
    // around the radar.  At this point, _oldSnr contains the SNR data
    // of the last radar file read in.

    float sum_pow = 0.0;

    int az;
    for (az = 0, index = r; az < _numAzimuth; ++az, index += _numGates)
    {
      sum_pow += pow(10.0, 0.1 * snr[index]);
    }
    
    // Calculate the side power

    float side_pow = sum_pow * pow(10.0, 0.1*side_lobe_pow);

    for (az = 0, index = r; az < _numAzimuth; ++az, index += _numGates)
    {
      if (_data[index] > 0.0)
      {
	float side_correction = exp(-side_pow*pow(10.0, -0.1*strength[index]));
	_data[index] *= side_correction;
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::quality(const FieldAlgo &ncp)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (ncp[i] != RefractInput::INVALID)
    {
      _data[i] = sqrt(RefractInput::SQR(ncp[i]));
    }
  }
}

//---------------------------------------------------------------------------
void FieldAlgo::phaseErr(const FieldAlgo &ncp)
{
  for (int i=0; i<_scanSize; ++i)
  {
    if (ncp[i] > 0.0)
    {
      _data[i] = sqrt(-2.0 * log(ncp[i]) / ncp[i]) / DEG_TO_RAD;
    }
    else
    {
      _data[i] = RefractInput::VERY_LARGE;
    }
  }
}
