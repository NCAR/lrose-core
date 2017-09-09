#include "Data.hh"
#include <Refract/FieldWithData.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/LogStream.hh>

const float Data::INVALID = -999999.0;
const float Data::VERY_LARGE = 2147483647;


static IQ _accumulateFarSector(int r, int num_gates, int num_azim,
			       const FieldDataPair &dif_prev_data)
{
  IQ ret;
  for (int dr = -num_gates/10; dr <= num_gates/10; ++dr)
  {
    if (r + dr < num_gates)
    {
      for (int daz = -num_azim/16; daz <= num_azim/16; ++daz)
      {
	int meas_az = daz;
	if (meas_az < 0)
	{
	  meas_az += num_azim;
	}
	if (meas_az >= num_azim)
	{
	  meas_az -= num_azim;
	}
	  
	int index = meas_az*num_gates + r + dr;
	ret += dif_prev_data[index];
      }
    }
  }
  return ret;
}

static void _adjustForAzimuth(int r, int az, int num_gates, int num_azim,
			      const FieldDataPair &dif_prev_data,
			      IQ &smooth_iq)
{
  for (int dr = -num_gates/10; dr <= num_gates/ 10; ++dr)
  {
    if (r + dr < num_gates)
    {
      int meas_az = az - 1 - (num_azim/16);
      if (meas_az < 0)
      {
	meas_az += num_azim;
      }
      int index = meas_az*num_gates + r + dr;
      smooth_iq -= dif_prev_data[index];

      meas_az = az + (num_azim/16);
      if (meas_az >= num_azim)
      {
	meas_az -= num_azim;
      }
      index = meas_az*num_gates + r + dr;
      smooth_iq += dif_prev_data[index];
    }
  }
}


//-----------------------------------------------------------------------
bool Data::gateSpacingChange(const FieldDataPair &iq)
{
  if (iq.wrongGateSpacing(_calibStrength.gateSpacing()))
  {
    LOG(ERROR) << "Wrong gate spacing";
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------------
double Data::getGateSpacing(void) const
{
  return _calibStrength.gateSpacing();
}

//-----------------------------------------------------------------------
void Data::allocate(const FieldDataPair &iq)
{
  _clear();
  
  int scan_size = iq.scanSize();
  
  // Allocate the global fields

  _fluctSnr = new double[scan_size];
  memset(_fluctSnr, 0, scan_size * sizeof(double));

  _pixelCount = new int[scan_size];
  memset(_pixelCount, 0, scan_size * sizeof(int));

  _meanSnr = iq.createField("mean_snr", "dB", 0.0);
  _sumP = iq.createField("sum_p", "none", 0.0);
  _calibStrength = iq.createField("strength", "none", 0.0);
  _calibPhaseEr = iq.createField("phase_er", "none", Data::VERY_LARGE);
  _calibQuality = iq.createField("quality", "none", 0.0);
  _calibNcp = iq.createField("ncp", "none", 0.0);
 
  _sumAB = FieldDataPair(iq, "sum_a", "none", 0.0, "sum_b", "none", 0.0);
  _calibAvIQ = FieldDataPair(iq, "av_i", "none", 0.0, "av_q", "none", 0.0);
  _avIQ = FieldDataPair(iq, "av_i", "none", "av_q", "none");  // init to IQ
  _difFromRefIQ = FieldDataPair(iq, "dif_from_ref_i", "none",
				iq.missingValueI(),
				"dif_from_ref_q", "none",
				iq.missingValueQ());
}

//-----------------------------------------------------------------------
void Data::accumulateABP(const FieldDataPair &difPrevScan)
{
  _sumAB += difPrevScan;
  _sumP.incrementNorm(difPrevScan);
}

//-----------------------------------------------------------------------
void Data::accumulateAB(const FieldDataPair &iq)
{
  _sumAB += iq;
}

//-----------------------------------------------------------------------
void Data::adjustmentsForSNR(const FieldAlgo &oldSNR,
			     const FieldWithData &SNR)
{
  int scan_size = oldSNR.data_size();
  for (int index = 0; index < scan_size; ++index)
  {
    if ((!SNR.isBadAtIndex(index)) && !oldSNR.isBadAtIndex(index))
    {
      _fluctSnr[index] += fabs(SNR[index] - oldSNR[index]);
      _meanSnr[index] += SNR[index];
      _pixelCount[index]++;
    }
  }
}

//-----------------------------------------------------------------------
void Data::initialCalibration(int r_min)
{
  _calibStrength.setStrength(_meanSnr, _pixelCount);
  
  int scan_size = _sumAB.scanSize();
  for (int i=0; i<scan_size; ++i)
  {
    if (_pixelCount[i] != 0)
    {
      _fluctSnr[i] /= static_cast<double>(_pixelCount[i]);
    }
  }
  _calibNcp.setNcp(_calibStrength, _sumAB, _sumP, _fluctSnr,
		   _pixelCount, r_min);

  // do this because there is a test in the side lobe filtering
  _calibPhaseEr.phaseErr(_calibNcp);
}  

//-----------------------------------------------------------------------
void Data::sideLobeFiltering(double beam_width, int r_min,
			     double side_lobe_pow,
			     const FieldAlgo &oldSnr)
{
  double nazim = static_cast<double>(_calibNcp.num_azim());
  double contamin_pow = exp(log(4.0) * Data::SQR(360.0 /nazim/beam_width));

  _calibNcp.sideLobeFilter(_calibStrength, r_min, contamin_pow);
  _calibPhaseEr.phaseErr(_calibNcp);

  
  // #### Assumption: PulseLength = GateSpacing with a matched filter
  // Then, look for possible range sidelobes
  contamin_pow = 4.0;
  _calibNcp.rangeSidelobeFilter(_calibStrength, r_min, contamin_pow);
  _calibPhaseEr.phaseErr(_calibNcp);

  
  // Finally handle 360 deg sidelobes
  LOG(DEBUG) << "Using side lobe power parameter " << side_lobe_pow;
  _calibNcp.sidelobe360Filter(_calibStrength, oldSnr, r_min, side_lobe_pow);
  _calibPhaseEr.phaseErr(_calibNcp);
}

//-----------------------------------------------------------------------
void Data::calculateQuality(void)
{
  _calibQuality.quality(_calibNcp);
}

//-----------------------------------------------------------------------
void Data::setABToZero(void)
{
  _sumAB.setAllZero();
}

//-----------------------------------------------------------------------
void Data::normalizeReference(void)
{
  int scanSize = _sumAB.scanSize();
  for (int i=0; i<scanSize; ++i)
  {
    _calibAvIQ[i].setToZero();
    if (_calibNcp[i] == INVALID)
    {
      continue;
    }
    float norm2 = sqrt(SQR(_calibNcp[i]));
    if (norm2 == 0.0)
    {
      continue;
    }
    
    float norm = _sumAB[i].norm() / norm2;

    if (norm == 0.0)
    {
      _calibPhaseEr[i] = VERY_LARGE;
    }
    else
    {
      _calibAvIQ[i] = _sumAB[i] / norm;
    }
  } 
}

//-----------------------------------------------------------------------
void Data::findReliableTargets(int r_min,  int r_max, const FieldDataPair &iq,
			       const FieldWithData &SNR,
			       FieldDataPair &difPrevScan)
{
  // put the iq into averageIQ
  _calibAvIQ += iq;

  // difPrevScan.normalizedDiff(iq, SNR);

  // Smooth (a lot!) _difPrevScan in dif_from_ref.  As a result,
  // dif_from_ref will map changes of phase over large areas (due to
  // meteorology) which could then be compensated for later.  First do it
  // at close range (average across all azimuths and a few range gates).

  // accumulate for azimuth=0 into the difFromFrefIQ using differences data
  // at this range over all azimuths
  int num_azim = iq.numAzim();
  int num_gates = iq.numGates();
  for (int r = r_min; r <= r_max; ++r)
  {
    _difFromRefIQ[r].setToZero();  //data.setToZero(r);
    for (int az = 0, index = r; az < num_azim; ++az, index += num_gates)
    {
      _difFromRefIQ[r] += difPrevScan[index];
    }
  }

  // take 3 point normalized average at each r for this data,
  // and write it for all other azimuths, then shift it down by one azimuth
  for (int r = r_min; r < r_max; ++r)
  {
    IQ p = _difFromRefIQ[r-1] + _difFromRefIQ[r] + _difFromRefIQ[r+1];
    p.normalize();

    for (int az = 1, index = num_gates + r; az < num_azim; 
	 ++az, index += num_gates)
    {
      _difFromRefIQ[index] = p;
    }
  }
  for (int r = r_min; r < r_max; ++r)
  {
    _difFromRefIQ[r] = _difFromRefIQ[r+num_gates];
  }

  // _difFromRefIQ.ave3PtFromZeroBeamIntoOtherBeams(r_min, r_max);

  // And then to ranges further away (sector average).
  for (int r = r_max; r < num_gates; ++r)
  {
    _difFromRefIQ[r] = _accumulateFarSector(r, num_gates, num_azim,
					    difPrevScan);
    for (int az = 1; az < num_azim; ++az)
    {
      _adjustForAzimuth(r, az, num_gates, num_azim, difPrevScan,
			_difFromRefIQ[r]);
	
      int index = az*num_gates + r;
      _difFromRefIQ[index] = _difFromRefIQ[r];
    }

    for (int az = 0; az < num_azim; ++az)
    {
      int index = az*num_gates + r;
      _difFromRefIQ[index].normalize();
    }
  }
  // info.smoothFarAway(max_r_smooth, _difPrevScan);

  // Smooth phase field done.  Now compensate the previously computed
  // phase difference field for the average change caused by meteorology
  // (i.e. the smoothed phase field).
  difPrevScan.phaseDiff2V(_difFromRefIQ);
}

//-----------------------------------------------------------------------
void Data::addCalibFields(DsMdvx &calib_file)
{
  calib_file.addField(_calibStrength.fieldCopy());
  _calibAvIQ.addToOutput(calib_file);
  calib_file.addField(_calibPhaseEr.fieldCopy());
  calib_file.addField(_calibQuality.fieldCopy());
  calib_file.addField(_calibNcp.fieldCopy());
}

//-----------------------------------------------------------------------
void Data::addDebug(DsMdvx &debug_file, bool first) const
{
  if (first)
  {
    debug_file.addField(_meanSnr.fieldCopy());
    _avIQ.addToOutput(debug_file);
  }
  else
  {
    debug_file.addField(_meanSnr.fieldCopy());
    _avIQ.addToOutput(debug_file);
    _difFromRefIQ.addToOutput(debug_file);
    _sumAB.addToOutput(debug_file);
    debug_file.addField(_sumP.fieldCopy());
  }
}

#ifdef NOTDEF
//-----------------------------------------------------------
double *Data::setPhaseTargetData(void) const
{
  double *ret = new double[_scan_size];
  for (int i=0; i<_scan_size; ++i)
  {
    if (_avIQ[i].isZero())
    {
      ret[i] = Data::INVALID;
    }
    else
    {
      ret[i] = _avIQ[i].phase();
    }
  }
  return ret;
}
#endif
  
//-----------------------------------------------------------------------
void Data::_clear(void)
{
  if (_fluctSnr != NULL)
  {
    delete [] _fluctSnr;
    _fluctSnr = NULL;
  }
  if (_pixelCount != NULL)
  {
    delete [] _pixelCount;
    _pixelCount = NULL;
  }

  _sumAB = FieldDataPair();
  _meanSnr = FieldAlgo();
  _sumP = FieldAlgo();
  _calibStrength = FieldAlgo();
  _calibAvIQ = FieldDataPair();
  _calibPhaseEr = FieldAlgo();
  _calibQuality = FieldAlgo();
  _calibNcp = FieldAlgo();
}


