// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 *
 * @file PhaseFit.hh
 *
 * @class PhaseFit
 *
 * Class for performing the phase fitting algorithm.
 *  
 * @date 3/29/2010
 *
 */

#ifndef PhaseFit_HH
#define PhaseFit_HH

#include <Refract/VectorIQ.hh>
#include <Refract/VectorData.hh>
#include <dataport/port_types.h>
#include <vector>

class TargetVector;
class FieldDataPair;
class VectorData;
class LinearInterpArgs;

/** 
 * @class PhaseFit
 */

class PhaseFit
{
 public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Constructor
   */

  PhaseFit();
  
  /**
   * @brief Destructor
   */

  virtual ~PhaseFit();
  
  /**
   * @brief Initialize the object.
   *
   * @param[in] num_beams Number of beams in the data.
   * @param[in] num_gates Number of gates in the data.
   * @param[in] gate_spacing Gate spacing in meters.
   * @param[in] wavelength The wavelength for the radar.
   * @param[in] min_consistency Minimum consistency of phase to accept
   *                            the measurement.  Higher means smaller
   *                            coverage of better data.
   * @param[in] r_min Minimum rage get of ground echo.
   *
   * @return Returns true on success, false on failure.
   */

  bool init(const int num_beams, const int num_gates, const double gate_spacing,
	    const double wavelength, const double min_consistency,
	    const int r_min);

  void initFit(const TargetVector &target, const FieldDataPair &_difPrevScan,
	       bool isPhaseDiff);
  
  /**
   * @brief Smooth the given phase field and compute the related N-field and
   *        its error.  Before calling this method, the caller must call
   *        the init() method, set the values in the phase, quality, inphase
   *        and quadrature arrays and call the following access methods:
   *        setRefN(), setSmoothSideLen(), setNOutput() and setNError().
   *
   * @param[in] do_relax Flag indicating whether to perform the relaxation
   *                     step.
   *
   * @return Returns true on success, false on failure.
   */

  bool fitPhaseField(const bool do_relax);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Set the reference N value to use in the calculations.
   */

  void setRefN(const double ref_n)
  {
    _refN = ref_n;
  }
  

  /**
   * @brief Set the smoothing side length value to use in the calculations.
   * and set derived member values
   */

  void setSmoothSideLen(const double smooth_sidelen)
  {
    _smoothSideLen = smooth_sidelen;
    _smoothRange = (int)(_smoothSideLen / 2.0 / _gateSpacing);
    if (_smoothRange <= 0)
      _smoothRange = 1;
    _twoSmoothRange = 2*_smoothRange;
  }
  
  /**
   * @brief Set the N output pointer.  This is the pointer to the location
   *        where the calculated N values will be written.  This array must
   *        be allocated by the calling method before this call and must
   *        be deleted by the calling method when no longer needed.
   */

  void setNOutput(fl32 *n_output)
  {
    _nOutput = n_output;
  }
  
  /**
   * @brief Set the N error pointer.  This is the pointer to the location
   *        where the calculated N error values will be written.  This array
   *        must be allocated by the calling method before this call and must
   *        be deleted by the calling method when no longer needed.
   */

  void setNError(fl32 *n_error)
  {
    _nError = n_error;
  }
  
  /**
   * Debug test
   */
  bool outOfBounds(int offset) const;
  /**
   * Debug test
   */
  bool outOfBounds(int max, int offset) const;

 private:

  /**
   * @brief Number of azimuths to smooth over when calculating the average
   *        slope.
   */

  static const int SMEAR_AZ;

  /**
   * @brief Number of azimuths to smooth over when calculating the initial
   *        slope.
   */

  static const int SMEAR_AZ_INIT;

  /**
   * @brief Number of bins to smooth over when calculating the average slope.
   */

  static const int SMEAR_RA;

  /**
   * @brief Number of meters to use in the initial slope calculation.
   */

  static const int INITIAL_SLOPE_LEN_M;

  /**
   * @brief Minimum number of iterations.
   */

  static const int MIN_ITER;
  
  /**
   * @brief Maximum number of iterations.
   */

  static const int MAX_ITER;
  
  /**
   * @brief Minimum absolute consistency value.
   */

  static const double MIN_ABS_CONSISTENCY;


  int _scanSize;            /**< Size of scan */

  /**
   * @brief Number of beams in the data.
   */

  int _numBeams;
  
  /**
   * @brief Number of gates in the data.
   */

  int _numGates;
  
  /**
   * @brief Gate spacing in meters
   */

  double _gateSpacing;
  
  /**
   * @brief Wavelength of data.
   */

  double _wavelength;
  
  /**
   * @brief Minimum consistency of phase to accept N (DN) measurement.
   *        Higher means smaller coverage of (hopefully) better data.
   */

  double _minConsistency;
  
  /**
   * @brief Minimum range gate of ground echo.
   */

  int _rMin;
  
  /**
   * @brief Azimuth spacing in degrees.
   */

  double _azimSpacing;
  
  /**
   * @brief Reference N value.
   */
  
  double _refN;

  /**
   * @brief phase data to fit. 
   */
  VectorData _phase;

  /**
   * @brief quality data to fit. 
   */
  VectorData _quality;

  /**
   * @brief IQ data.
   */
  VectorIQ _iq;

  /**
   * @brief Smoothed phase field.
   */
  VectorData _phaseFit;

  /**
   * @brief Error in the smoothed phase field.
   */
  VectorData _phaseError;

  /**
   * @brief I component of the smoothed phase field.
   */
  VectorIQ _smoothIQ;

  /**
   * @brief Calculated N values.  This pointer points to space not owned by
   *        this object and must not be deleted by this object.
   */
  fl32 *_nOutput;

  /**
   * @brief Error in the calculated N value.  This is a function of sigma(N)
   *        and the input data density/quality.  This pointer points to space
   *        not owned by this object and must not be deleted by this object.
   */
  fl32 *_nError;

  /**
   * @brief Expected phase for 1st range.
   */
  double _expectedPhaseRange0;

  /**
   * @brief Average d(Phase)/d(range)
   */
  double _rangeSlope;

  /**
   * @brief Smoothing side length in meters.
   */
  double _smoothSideLen;

  int _smoothRange;     /**< Used in smoothing */
  int _twoSmoothRange;  /**< Used in smoothing */

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Smooth the phase measurements of the map information loaded
   *        in this object.
   *
   * @return Returns the initial range slope value for this scan.  Returns
   *         INVALID on error.
   */

  float _doSmoothing(double phaseSlopeInit);
  void _doSmoothingRange(int r, const VectorData &slope_in_range,
			 VectorData &next_slope_in_range,
			 VectorIQ &slope_ab,
			 float range_slope, float init_slope);
  

  /**
   * @brief Compute average slope of phase data with range 
   *   using a pulse-pair method.
   *   Output: Average slope in degrees per gatespacing.
   *   Called by: _fitPhases() and _doSmoothing()
   *
   * @param[in] inphase The I values.
   * @param[in] quadrature The Q values.
   * @param[in] num_azim Number of azimuths in data.
   * @param[in] num_range Number of ranges in data.
   * @param[in] range0 First useful range bin.
   * @param[in] max_r
   * @param[in] smear_az
   *
   * @return Returns the average slope of the phase data.  Returns INVALID
   *         on error.
   */

  double _meanPhaseSlope(const int max_r, const int smear_az) const;
  

  /**
   * @brief Compute the average slope of phase data with range using a
   *        pulse-pair method.
   *
   * @return Returns the calculated slope value.  Returns INVALID on
   *         error.
   */

  double _meanPhaseSlopeAvg() const;
  

  /**
   * @brief Compute the initial slope of phase data with range using a
   *        pulse-pair method.
   *
   * @return Returns the calculated slope value.  Returns INVALID on
   *         error.
   */

  double _meanPhaseSlopeInit() const;
  
  IQ _sectorMeanPhaseSlope(int az, int max_r, int smear_az) const;
  // void _meanPhaseSlopeIncrementAtAz(int az, int max_r, int smear_az,
  // 				    IQ &slope_iq) const;
  IQ _meanPhaseSlopeAtRange(int r, int offset, int smear_az) const;

  /**
   * @brief Evaluate the phase at range RMin.  Since it is at very close
   *        range, azimuthal dependance of phase is unlikely and it provides a
   *        needed starting point to compute N from d(Phi)/dr.
   *
   * @return Returns the calculated phase value.
   */

  double _phaseRange0();
  
  void _setNOneBeam(int azn, float slope_to_n, float tmp, float er_decorel);

  float _guessPhase(int az, int r, float slope_in_range, int &rjump) const;

  void _linearInterp(const LinearInterpArgs &args,
		     const VectorData &slope_in_range,
		     const VectorIQ &slope_ab, const VectorData &guess_phase,
		     VectorIQ &sum_iq, VectorData &max_quality,
		     VectorData &next_slope_in_range);
  double _linearInterpUpdateMaxQual(const LinearInterpArgs &args, int j) const;
  IQ _linearInterpUpdateSumIQ(const LinearInterpArgs &args, int j,
			      const VectorData &slope_in_range,
			      const VectorIQ &slope_ab) const;
  void _linearInterpIncConsistency(int daz, const LinearInterpArgs &args,
				   const VectorIQ &sum_iq,
				   const VectorIQ &slope_ab,
				   const VectorData &guess_phase,
				   const VectorData &max_quality,
				   IQ &tmp_ab, float &max_consistency) const;
  void _setConsistencyAndQuality(const IQ &tmp_ab,
				 float weight_fact,
				 float maxconsistency,
				 float &consistency,
				 float &quality) const;
void  _setPhaseAndNextSlopeInRange(const LinearInterpArgs &args,
				   const IQ &tmp_ab,
				   const VectorData &guess_phase,
				   float quality, float consistency,
				   float maxconsistency,
				   const VectorData &slope_in_range,
				   VectorData &next_slope_in_range);
void _resetSmoothIQAndNextSlopeInRange(const LinearInterpArgs &args,
				       const VectorData &guess_phase,
				       const VectorData &slope_in_range,
				       VectorData &next_slope_in_range);
void _lowQualityAdjust(const LinearInterpArgs &args,
		       VectorData &next_slope_in_range);

  /**
   * @brief Perform a quality-dependent iterative smoothing of the
   *        refractivity field, constrained by the smoothed phase field.
   *   Uses an iterative "diffusion and relaxation" approach to the problem, 
   *   even if it takes an eternity:
   *   First the value of N are nudged to match better the phase data,
   *   then the field is smoothed.  As a result, the regions where data are
   *   available gradually percolate into regions where they are not.
   *   Input: Phases_to_fit structure and initial N field data.
   *   Output: Updated N field data.
   *   Called by: fit_phase_field().
   */

  int _relax(float *ndata);
  
  inline double _defaultSlope() const
  {
    return 1000000.0 / _gateSpacing * _wavelength / 720.0;
  }

};


#endif
