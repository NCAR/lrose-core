#include <Refract/FieldWithData.hh>
#include <Refract/FieldDataPair.hh>
#include <Refract/RefractInput.hh>
#include <Refract/RefractConstants.hh>
#include <Refract/RefractUtil.hh>
#include <toolsa/LogStream.hh>
#include <toolsa/str.h>
#include <toolsa/toolsa_macros.h>
#include <cmath>

//-----------------------------------------------------------------------
FieldWithData::FieldWithData(const MdvxField *f, const std::string &name,
			     const std::string &units, double value) :
  _name(name)
{
  if (f == NULL)
  {
    _field = NULL;
    _data = NULL;
  }
  else
  {
    _field = _createMatchingField(f, name, units);
    _field_hdr = _field->getFieldHeader();
    _data = (fl32 *)_field->getVol();
    for (int i=0; i<_field_hdr.nx*_field_hdr.ny; ++i)
    {
      _data[i] = value;
    }
  }
}

//-----------------------------------------------------------------------
FieldWithData::FieldWithData(const MdvxField *f, const std::string &name,
			     const std::string &units) :
  _name(name)
{
  if (f == NULL)
  {
    _field = NULL;
    _data = NULL;
  }
  else
  {
    _field = _createMatchingField(f, name, units);
    _field_hdr = _field->getFieldHeader();
    _data = (fl32 *)_field->getVol();
  }
}

//-----------------------------------------------------------------------
FieldWithData::FieldWithData(const FieldWithData &f, const std::string &name,
			     const std::string &units) :
  _name(name)
{
  if (f._field == NULL)
  {
    _field = NULL;
    _data = NULL;
  }
  else
  {
    _field = _createMatchingField(f._field, name, units);
    _field_hdr = _field->getFieldHeader();
    _data = (fl32 *)_field->getVol();
  }
}

//-----------------------------------------------------------------------
FieldWithData::FieldWithData(const FieldWithData &inp0, double w0,
			     const FieldWithData &inp1, double w1) :
  _field(NULL), _data(NULL)
{
  if (w0 <= 0 && w1 > 0)
  {
    *this = inp1;
  }
  else if (w1 <= 0 && w0 > 0)
  {
    *this = inp0;
  }
  else
  {
    *this = inp0;
    for (int i = 0; i < _field_hdr.nx*_field_hdr.ny; ++i)
    {
      if (inp0.isBadAtIndex(i))
      {
	if (inp1.isBadAtIndex(i))
	{
	  _data[i] = inp0._field_hdr.missing_data_value;
	}
	else
	{
	  _data[i] = inp1._data[i];
	}
      }
      else
      {
	if (inp1.isBadAtIndex(i))
	{
	  _data[i] = inp0._data[i];
	}
	else
	{
	  _data[i] = inp0._data[i]*w0 + inp1._data[i]*w1;
	}
      }
    }
  }
}

//-----------------------------------------------------------------------
FieldWithData::FieldWithData(const MdvxField *f)
{
  if (f == NULL)
  {
    _field = NULL;
    _data = NULL;
    _name = "";
  }
  else
  {
    _field = new MdvxField(*f);
    _field_hdr = _field->getFieldHeader();
    _data = (fl32 *)_field->getVol();
    _name = _field_hdr.field_name;
  }
}

//-----------------------------------------------------------------------
MdvxPjg FieldWithData::createProj(void) const
{
  return MdvxPjg(_field_hdr);
}

//-----------------------------------------------------------------------
MdvxField *FieldWithData::
createMatchingBlankField(const std::string &name) const
{
  MdvxPjg proj = createProj();
  double elevation = getElev();
  return createBlankField(proj, name, elevation);
}

//-----------------------------------------------------------------------
MdvxField *FieldWithData::createMatchingField(void) const
{
  // Create the new field header
  Mdvx::field_header_t field_hdr = _field->getFieldHeader();
  
  // Create the new vlevel header
  Mdvx::vlevel_header_t vlevel_hdr = _field->getVlevelHeader();

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = refract::INVALID;
  field_hdr.missing_data_value = refract::INVALID;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  
  // Create the new field
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}

//-----------------------------------------------------------------------
MdvxField *FieldWithData::createMatchingField(const std::string &name,
					      const std::string &units) const
{
  // Create the new field header

  Mdvx::field_header_t field_hdr = _field->getFieldHeader();
  
  // Create the new vlevel header

  Mdvx::vlevel_header_t vlevel_hdr = _field->getVlevelHeader();

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = refract::INVALID;
  field_hdr.missing_data_value = refract::INVALID;
  field_hdr.min_value = 0.0;
  field_hdr.max_value = 0.0;
  STRcopy(field_hdr.field_name_long, name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Create the new field
  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}

//-----------------------------------------------------------------------
MdvxField *
FieldWithData::createMatchingField(const std::string &field_name,
				   const std::string &units,
				   double initial_value) const
{
  // Create the field
  MdvxField *field = createMatchingField(field_name, units);
  if (field == 0)
    return 0;
  
  // Set the initial data values
  fl32 *data = (fl32 *)field->getVol();
  for (int i = 0; i < _field_hdr.nx*_field_hdr.ny; ++i)
    data[i] = initial_value;
  return field;
}

//-----------------------------------------------------------------------
MdvxField *FieldWithData::fieldCopy(void) const
{
  if (_field == NULL)
  {
    return NULL;
  }
  else
  {
    return new MdvxField(*(_field));
  }
}

//-----------------------------------------------------------------------
MdvxField *
FieldWithData::createMatchingFieldWithData(const std::string &field_name,
					   const std::string &units) const
{
  // Create the field
  MdvxField *field = createMatchingField(field_name, units);
  if (field == 0)
    return 0;
  
  // Set the initial data values
  fl32 *data = (fl32 *)field->getVol();
  for (int i = 0; i < _field_hdr.nx*_field_hdr.ny; ++i)
    data[i] = _data[i];
  return field;
}

//-----------------------------------------------------------------------
MdvxField *
FieldWithData::createMatchingFieldWithData(void) const
{
  // Create the field
  MdvxField *field = createMatchingField();
  if (field == 0)
    return 0;
  
  // Set the initial data values
  fl32 *data = (fl32 *)field->getVol();
  for (int i = 0; i < _field_hdr.nx*_field_hdr.ny; ++i)
    data[i] = _data[i];
  return field;
}

//-----------------------------------------------------------------------
bool FieldWithData::isBadAtIndex(int index) const
{
  return (_data[index] == _field_hdr.missing_data_value ||
	  _data[index] == _field_hdr.bad_data_value);
}

//-----------------------------------------------------------------------
bool FieldWithData::wrongGateSpacing(double wanted) const
{
  if (_field_hdr.grid_dx != wanted)
  {
    LOG(ERROR) << _name << " Gate spacing changed was " << wanted
	       << " now " << _field_hdr.grid_dx;
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------------
void FieldWithData::setAllZero(void) const
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int i=0; i< scan_size; ++i)
  {
    _data[i] = 0.0;
  }
}

//-----------------------------------------------------------------------
std::vector<double>  FieldWithData::productVector(int *count) const
{
  vector<double> ret;
  
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  if (scan_size == 0)
  {
    return ret; // NULL;
  }
  ret.reserve(scan_size);
  // double *ret = new double[scan_size];
  for (int i=0; i< scan_size; ++i)
  {
    ret.push_back(_data[i]*(double)count[i]);
    // ret[i] = _data[i]*(double)count[i];
  }
  return ret;
}

//-----------------------------------------------------------------------
void FieldWithData::mask(const FieldWithData &mask, double maskValue,
			 double replaceValue)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int i=0; i< scan_size; ++i)
  {
    if (mask._data[i] == maskValue)
    {
      _data[i] = replaceValue;
    }
  }
}

//-----------------------------------------------------------------------
void FieldWithData::maskWhenCountNonPositive(const int *count,
					     double replaceValue)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int i=0; i< scan_size; ++i)
  {
    if (count[i] <= 0)
    {
      _data[i] = replaceValue;
    }
  }
}

//-----------------------------------------------------------------------
void FieldWithData::setStrength(const FieldWithData &meanSnr, const int *count,
				const std::vector<int> &imask, double invalid)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int i=0; i< scan_size; ++i)
  {
    if (imask[i] != 0 && count[i] != 0)
    {
      _data[i] = meanSnr[i] / (double)count[i];
    }
    else
    {
      _data[i] = invalid;
    }
  }
}

//-----------------------------------------------------------------------
void FieldWithData::setNcp(const FieldDataPair &sumAB,
			   const std::vector<double> &denom,
			   int rMin, const int *count,
			   const FieldWithData &strength,
			   const float *fluctSnr)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int i=0; i< scan_size; ++i)
  {
    if (denom[i] == 0.0 || count[i] == 0)
    {
      continue;
    }
    // double ncp = sumAB.sumSquares(i)/denom[i];
    double ncp = sumAB[i].normSquared()/denom[i];
    int r = rIndex(i, _field_hdr.ny, _field_hdr.nx);
    if (ncp != 0.0 && r >= rMin)
    {
      if (count[i] > 1)
      {
	ncp = ((ncp - 1.0/sqrt((double)count[i])) /
	       (1.0-1.0/sqrt((double)count[i])));
	if (ncp < 0.001) ncp = 0.001;
	if (ncp > 0.9999) ncp = 0.9999;
      }
      else
      {
	ncp = 0.5;
      }
      ncp *= 1.0/(1.0+pow(10.0, -0.1*strength[i]));
      double strength_correct = exp(-0.001*pow(fluctSnr[i], 4.0))/exp(-0.002);
      if (strength_correct > 1.0) strength_correct = 1.0;
      if (strength_correct < 0.1) strength_correct = 0.1;
      ncp *= strength_correct;
      if (ncp < 0.0) ncp = 0.0;
      _data[i] = ncp;
    }
  }
}

//-----------------------------------------------------------------------
void FieldWithData::setPhaseErr(const FieldWithData &ncp,
				const std::vector<int> &imask, int rMin,
				double very_large)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int i=0; i< scan_size; ++i)
  {
    int r = rIndex(i, _field_hdr.ny, _field_hdr.nx);
    if (imask[i] == 0 || r < rMin)
    {
      continue;
    }
    if (ncp[i] > 0.0)
    {
      _data[i] = sqrt(-2.0 * log(ncp[i]) / ncp[i]) / DEG_TO_RAD;
    }
    else
      _data[i] = very_large;
  }
}

//-----------------------------------------------------------------------
//double *FieldWithData::normalizationArray(double invalid) const
std::vector<double> FieldWithData::normalizationVector(double invalid) const
{
  vector<double> ret;
  
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  if (scan_size <= 0)
  {
    return ret;
    // return NULL;
  }
  ret.reserve(scan_size);
  // double *ret = new double[scan_size];
  for (int i=0; i< scan_size; ++i)
  {
    if (_data[i] == invalid)
    {
      ret.push_back(-1.0);
    }
    else if (_data[i] == 0.0)
    {
      ret.push_back(-1.0);
    }
    else
    {
      ret.push_back(fabs(_data[i]));
    }
  }
  return ret;
}

//-----------------------------------------------------------------------
void FieldWithData::mainlobeElimination(int rmin, double contamin_pow,
					const FieldWithData &strengthField)
{
  int num_azim = _field_hdr.ny;
  int num_gates = _field_hdr.nx;

  for (int az = 0, index = 0; az < num_azim; ++az)
  {
    int r;

    for (r = rmin, index += rmin; r < num_gates; ++r, ++index)
    {
      if (_data[index] > 0.0)
      {
	// Get the power at the current gate

	float local_pow = pow(10.0, 0.1 * strengthField[index]);

	// Get the power at the gates in the surrounding beams

	int prev_az = (az + num_azim - 1) % num_azim;
	float near_pow =
	  pow(10.0, 0.1 * strengthField[prev_az * num_gates + r]);

	int next_az = (az + 1) % num_azim;
	near_pow +=
	  pow(10.0, 0.1 * strengthField[next_az * num_gates + r]) ;

	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  float side_correction =
	    exp(-0.5 * RefractUtil::SQR((near_pow - 2.5 * contamin_pow) /
			   (2.5 * contamin_pow)));
	  if (side_correction < 0.1)
	    side_correction = 0.1;

	  _data[index] *= side_correction;
	}
      }
    } /* endfor - r */
  } /* endfor - az, index */
}

//---------------------------------------------------------------------------
void FieldWithData::rangeSidelobeElimination(int rMin, double contamin_pow,
					     double veryLarge,
					     const FieldWithData &strengthField)
{
  int num_azim = _field_hdr.ny;
  int num_gates = _field_hdr.nx;

  for (int az = 0, index = 0; az < num_azim; ++az)
  {
    int r;
    for (r = rMin, index += rMin; r < num_gates; ++r, ++index)
    {
      if (_data[index] > 0.0)
      {
	// Get the power at the current gate
	float local_pow = pow(10.0, 0.1 * strengthField[index]);

	// Get the power at the surrounding gates along this beam

	float near_pow;
	
	if (r == 0)
	  near_pow = veryLarge;
	else
	  near_pow = pow(10.0, 0.1 * strengthField[index-1]);

	if (r < num_gates - 1)
	  near_pow += pow(10.0, 0.1 * strengthField[index+1]);

	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  float side_correction =
	    exp(-0.5 * RefractUtil::SQR((near_pow - 2.5 * contamin_pow) /
			   (2.5 * contamin_pow)));
	  if (side_correction < 0.1)
	    side_correction = 0.1;
	  _data[index] *= side_correction;
	}
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::sidelobe360Elimination(int rMin, double sideLobePower,
					   const FieldWithData &strengthField,
					   const FieldWithData &oldSnrField)
{  
  int num_azim = _field_hdr.ny;
  int num_gates = _field_hdr.nx;

  for (int r = rMin; r < num_gates; ++r)
  {
    // Sum the power values at all gates the same distance as this gate
    // around the radar.  At this point, _oldSnr contains the SNR data
    // of the last radar file read in.
    float sum_pow = 0.0;
    for (int az = 0, index = r; az < num_azim; ++az, index += num_gates)
      sum_pow += pow(10.0, 0.1 * oldSnrField[index]);

    // Calculate the side power
    float side_pow = sum_pow*pow(10.0, 0.1*sideLobePower);
    for (int az = 0, index = r; az < num_azim; ++az, index += num_gates)
    {
      if (_data[index] > 0.0)
      {
	float side_correction =
	  exp(-side_pow * pow(10.0, -0.1 * strengthField[index]));

	_data[index] *= side_correction;
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::setPhaseError(const FieldWithData &ncpField,
				  double very_large)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int index=0; index < scan_size; ++index)
  {
    if (ncpField[index] != 0.0)
    {
      _data[index] =
	sqrt(-2.0*log(ncpField[index])/ncpField[index])/DEG_TO_RAD;
    }
    else
    {
      _data[index] = very_large;
    }
  }
}

//---------------------------------------------------------------------------
std::vector<double> FieldWithData::setQualityVector(const FieldWithData &qual,
						    bool quality_from_width,
						    bool quality_from_cpa,
						    double thresh_width,
						    double abrupt_factor) const
{
  vector<double> ret;
  
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  if (scan_size <= 0)
  {
    return ret;
  }
  // double *ret = new double[scan_size];

  ret.reserve(scan_size);

  // local object is SNR data
  for (int i=0; i<scan_size; ++i)
  {
    if (!isBadAtIndex(i))
    {
      ret.push_back(1.0 / (1.0 + pow(10.0, - 0.1 * _data[i])));
    }
    else
    {
      ret.push_back(0.5);
    }
  }
  
  for (int i=0; i<scan_size; ++i)
  {
    if (!qual.isBadAtIndex(i))
    {
      if (quality_from_width)
      {
	ret[i] *= exp(-pow((qual[i]/thresh_width), abrupt_factor));
      }	  
      if (quality_from_cpa)
      {
	ret[i] *= qual[i];
      }
    }
  }
  return ret;
}


// side effect modify quality array in some cases
std::vector<double>
FieldWithData::setPhaseErVector(std::vector<double> &quality,
				const FieldDataPair &calib_av_iq,
				int minR) const
{
  vector<double> ret;
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  if (scan_size <= 0)
  {
    return ret;
  }

  // double *ret = new double[scan_size];
  ret.reserve(scan_size);

  for (int i=0; i<scan_size; ++i)
  {
    if (quality[i]>0.0 && _data[i] < refract::VERY_LARGE)
    {
      ret.push_back(sqrt(-2.0 * log( quality[i] ) / quality[i]) / DEG_TO_RAD);
    }
    else
    {
      ret.push_back(refract::VERY_LARGE);
      quality[i] = 0.0;
    }
  }
  
  for (int i=0; i<scan_size; ++i)
  {
    if (ret[i] < _data[i] && ret[i] != refract::VERY_LARGE)
    {
      int r = rIndex(i, _field_hdr.ny, _field_hdr.nx);
      if (quality[i] > 0.5 && _data[i] > 2000.0 && r > minR)
      {
	// Good-looking unknown distant targets (anoprop?) get minimum
	// recognition
	quality[i] *= 0.04;
	ret[i] = sqrt(-2.0 * log(quality[i]) / quality[i]) / DEG_TO_RAD;
      }
      else
      {
	quality[i] = calib_av_iq[i].norm();
	ret[i] = _data[i];
      }
    }
  }
  return ret;
}

//---------------------------------------------------------------------------
void FieldWithData::setAbsValue(const FieldWithData &inp, double invalid)
{
  int scan_size = _field_hdr.ny * _field_hdr.nx;
  for (int index=0; index < scan_size; ++index)
  {
    if (inp[index] != invalid)
    {
      _data[index] = fabs(inp[index]);
    }      
  } /* endfor - az */
}

//-----------------------------------------------------------------------
int FieldWithData::rIndex(int index, int numAz, int numGates)
{
  return index - (index/numGates)*numGates;
}

//-----------------------------------------------------------------------
MdvxField *FieldWithData::createBlankField(const MdvxPjg &proj,
					   const std::string &field_name,
					   double elevation)
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  proj.syncToFieldHdr(field_hdr);
  
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size = field_hdr.nx * field_hdr.ny * field_hdr.nz *
    field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  field_hdr.data_dimension = 2;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = refract::MISSING_DATA_VALUE;
  field_hdr.missing_data_value = refract::MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_ELEV;
  vlevel_hdr.level[0] = elevation;
  
  // Create the blank field

  return new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
}

#ifdef NOTYET
//---------------------------------------------------------------------------
void FieldWithData::incrementNorm(const FieldDataPair &data)
{
  for (int i=0; i<data_size(); ++i)
  {
    _data[i] += data.norm(i);
  }
}

//---------------------------------------------------------------------------
void FieldWithData::setStrength(const FieldWithData &sumP,
				const FieldWithData &meanSnrField,
				const int *pixelCount)
{
  for (int i=0; i<data_size(); ++i)
  {
    if (pixelCount[i] == 0 || sumP._data[i] == 0.0)
    {
      _data[i] = refract::INVALID;
    }
    else
    {
      _data[i] = meanSnrField._data[i]/static_cast<double>(pixelCount[i]);
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::setNcp(const FieldWithData &strength,
			   const FieldDataPair &sumAB,
			   const FieldWithData &sumP, const double *fluctSnr,
			   const int *pixelCount, int r_min)
{
  int _numAzimuth = _field_hdr.ny;
  int _numGates = _field_hdr.nx;

  int i = 0;
  for (int a=0; a<_numAzimuth; ++a)
  {
    for (int r=0; r<_numGates; ++r, ++i)
    {
      double fpix = pixelCount[i];
      double ncp = 0.0;
      if (sumP._data[i] != 0.0 && pixelCount[i] != 0)
      {
	ncp = sumAB.normSquared(i)/sumP._data[i]/fpix;
	if (ncp != 0.0 && r >= r_min)
	{
	  if (fpix > 1.0)
	  {
	    double sfpix = sqrt(fpix);
	    ncp = (ncp - 1.0/sfpix) / (1.0 - 1.0/sfpix);
	    if (ncp < 0.001) ncp = 0.001;
	    if (ncp > 0.999) ncp = 0.999;
	  }
	  else
	  {
	    ncp = 0.5;
	  }
      
	  ncp *= 1.0/(1.0 + pow(10.0, -0.1*strength._data[i]));

	  double strength_correction = exp(-0.001*pow(fluctSnr[i], 4.0))/
	    exp(-0.002);
	  if (strength_correction > 1.0) strength_correction = 1.0;
	  if (strength_correction < 0.1) strength_correction = 0.1;
	  ncp *= strength_correction;
	}
      }
      _data[i] = ncp;
    }
  }
}

//----------------------------------------------------------------------------
void FieldWithData::sideLobeFilter(const FieldWithData &strength, int r_min,
			       double contamin_pow)
{
  int _numAzimuth = _field_hdr.ny;
  int _numGates = _field_hdr.nx;

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

	double local_pow = pow(10.0, 0.1 * strength._data[index]);

	// Get the power at the gates in the surrounding beams

	int prev_az = (az + _numAzimuth- 1) % _numAzimuth;
	double near_pow = pow(10.0, 0.1*strength._data[prev_az*_numGates+r]);

	int next_az = (az + 1) % _numAzimuth;
	near_pow += pow(10.0, 0.1 * strength._data[next_az*_numGates + r]) ;

	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  double side_correction =_sideCorrection(near_pow, contamin_pow);
	  _data[index]  *= side_correction;
	}
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::rangeSidelobeFilter(const FieldWithData &strength,
					int r_min, double contamin_pow)
{
  int _numAzimuth = _field_hdr.ny;
  int _numGates = _field_hdr.nx;

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

	float local_pow = pow(10.0, 0.1 * strength._data[index]);

	// Get the power at the surrounding gates along this beam

	float near_pow;
	
	if (r == 0)
	{
	  near_pow = refract::VERY_LARGE;
	}
	else
	{
	  near_pow = pow(10.0, 0.1 * strength._data[index-1]);
	}
	
	if (r < _numGates - 1)
	{
	  near_pow += pow(10.0, 0.1 * strength._data[index+1]);
	}
	
	// See if we need to do the side correction

	near_pow /= local_pow;

	if (near_pow > 2.5 * contamin_pow)
	{
	  double side_correction =_sideCorrection(near_pow, contamin_pow);
	  // float side_correction =
	  //   exp(-0.5 *RefractUtil::SQR((near_pow - 2.5*contamin_pow) /(2.5*contamin_pow)));
	  // if (side_correction < 0.1)
	  // {
	  //   side_correction = 0.1;
	  // }
	  _data[index] *= side_correction;
	}
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::sidelobe360Filter(const FieldWithData &strength,
				  const FieldWithData &snr, int r_min,
				  double side_lobe_pow)
{
  int _numAzimuth = _field_hdr.ny;
  int _numGates = _field_hdr.nx;

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
	float side_correction =
	  exp(-side_pow*pow(10.0, -0.1*strength[index]));
	_data[index] *= side_correction;
      }
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::quality(const FieldWithData &ncp)
{
  int _scanSize = _field_hdr.ny*_field_hdr.nx;
  for (int i=0; i<_scanSize; ++i)
  {
    if (ncp._data[i] != refract::INVALID)
    {
      _data[i] = sqrt(RefractUtil::SQR(ncp._data[i]));
    }
  }
}

//---------------------------------------------------------------------------
void FieldWithData::phaseErr(const FieldWithData &ncp)
{
  int _scanSize = _field_hdr.ny*_field_hdr.nx;
  for (int i=0; i<_scanSize; ++i)
  {
    if (ncp._data[i] > 0.0)
    {
      _data[i] = sqrt(-2.0 * log(ncp._data[i]) / ncp._data[i]) / DEG_TO_RAD;
    }
    else
    {
      _data[i] = refract::VERY_LARGE;
    }
  }
}

//-----------------------------------------------------------------------
void FieldWithData::initMdvMasterHdr(const time_t &t, const std::string &info,
				     const std::string &dataSetName, 
				     const std::string &dataSetSource,
				     double nValue,  DsMdvx &mdv) const
{
  mdv = DsMdvx();
  if (_field == NULL)
  {
    return;
  }
  
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = t;
  master_hdr.time_end = t;
  master_hdr.time_centroid = t;
  master_hdr.time_expire = t;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_ELEV;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  master_hdr.user_data_fl32[0] = static_cast<fl32>(nValue);
  master_hdr.sensor_lon = _field_hdr.proj_origin_lon;
  master_hdr.sensor_lat = _field_hdr.proj_origin_lat;
  master_hdr.sensor_alt = 0.0;
  STRcopy(master_hdr.data_set_info, info.c_str(), MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, dataSetName.c_str(), MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, dataSetSource.c_str(), MDV_NAME_LEN);
  mdv.setMasterHeader(master_hdr);
}
#endif

//-----------------------------------------------------------------------
MdvxField *FieldWithData::_createMatchingField(const MdvxField *f,
					       const std::string &name,
					       const std::string &units) const
{
  // Create the new field header

  Mdvx::field_header_t field_hdr = f->getFieldHeader();
  
  // Create the new vlevel header

  Mdvx::vlevel_header_t vlevel_hdr = f->getVlevelHeader();

  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  STRcopy(field_hdr.field_name_long, name.c_str(), MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, units.c_str(), MDV_UNITS_LEN);
  
  // Create the new field
  MdvxField *ret = new MdvxField(field_hdr, vlevel_hdr, (void *)0, true);
  fl32 *rdata = (fl32 *)ret->getVol();
  fl32 *idata = (fl32 *)f->getVol();
  for (int i = 0; i < field_hdr.nx*field_hdr.ny; ++i)
    rdata[i] = idata[i];
  return ret;
}

double FieldWithData::_sideCorrection(double near_pow, double contamin_pow)
{
  double sideCorrection = 
    exp(-0.5 * RefractUtil::SQR((near_pow - 2.5*contamin_pow) /
				(2.5*contamin_pow)));
  if (sideCorrection < 0.1)
  {
    sideCorrection = 0.1;
  }
  return sideCorrection;
}
